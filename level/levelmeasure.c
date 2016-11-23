/*
 * levelmeasure.c
 *
 *  Created on: Jul 26, 2016
 *      Author: vnarmy
 */
#include <esp8266.h>
#include "levelmeasure.h"
#include "config.h"
#include "crc12.h"
#include "uart.h"
#include "mqtt.h"

#ifdef LEVEL_DBG
#define DBG(format, ...) do { os_printf(format, ## __VA_ARGS__); } while(0)
#else
#define DBG(format, ...) do { } while(0)
#endif

#define	CMD_BEGIN	0x68
#define	CMD_END		0x16
#define	MSG_LEN		10
#define LIQUID		0x50
#define	THERMO		0x51

const uint8_t FuelLevelCmd[] = {0x68, 0x05, 0x11, 0x08, 0x14, 0x50, 0x00, 0x0F, 0x9E, 0x16};
const uint8_t FuelAvgTemperCmd[] = {0x68, 0x05, 0x11, 0x08, 0x14, 0x51, 0x00, 0x0E, 0x9E, 0x16};
#define LEVEL_MSG_MAX 64

#define RESP_SZ 64
LOCAL char responseBuf[RESP_SZ]; // buffer to accumulate responses from optiboot
LOCAL uint8_t responseLen = 0;     // amount accumulated so far
LOCAL bool endofMsg = false;
LOCAL os_timer_t acq_timer;

extern MQTT_Client mqttClient; // main mqtt client used by esp-link
static void mqttStatusCb(void);

LOCAL uint8_t ICACHE_FLASH_ATTR
tank_msg_create ( char* buffer, uint8_t msgtype )
{
	os_memset(buffer,0,MSG_LEN);
	buffer[0] = CMD_BEGIN;
	buffer[1] = 5;
	os_memcpy((buffer+2), flashConfig.device_id,3);
	buffer[5] = msgtype;
	buffer[6] = 0x00;
	uint16_t crc12 = crc_check((uint8_t*)(buffer+2),5);
	buffer[7] = (uint8_t)(crc12 >> 8);
	buffer[8] = (uint8_t)(crc12 & 0x00FF);
	buffer[9] = 0x16;
	for(uint8_t i=0;i<10;++i)
		DBG("%02X",buffer[i]);
	DBG("\n");
	return(MSG_LEN);
}

LOCAL uint8_t ICACHE_FLASH_ATTR
tank_acq_data( uint8_t msgtype )
{
	char* outmessage = os_zalloc(MSG_LEN);
	tank_msg_create(outmessage, msgtype );
	msgRespBufferInit();
	uart0_tx_buffer(outmessage,MSG_LEN);
	return msgtype;
}
LOCAL void ICACHE_FLASH_ATTR
tank_acq_level(  )
{
	tank_acq_data(LIQUID);
}
void ICACHE_FLASH_ATTR
msgParsePacket(char *buf, uint8_t length)
{
	os_memcpy(tank_data.device_id,&buf[0],3);
	switch(buf[3])
	{
	case LIQUID:
		tank_data.level[0] = ((uint32_t)buf[4]<<16) | ((uint32_t)buf[5] << 8) | ((uint32_t)buf[6]);

		tank_data.level[1] = ((uint32_t)buf[7]<<16) | ((uint32_t)buf[8] << 8) | ((uint32_t)buf[9]);
#ifdef LEVEL_DBG
		os_printf("Fuel level: %lu, Water Level: %lu\r\n",tank_data.level[0],tank_data.level[1]);
#endif
		tank_acq_data(THERMO);
		break;
	case THERMO:
		tank_data.thermo = ((uint16_t)buf[4] << 8) | ((uint16_t)buf[5]);
#ifdef LEVEL_DBG
		os_printf("Thermo: %u\r\n",tank_data.thermo);
#endif
		mqttStatusCb();
		break;
	}

}
// callback with a buffer of characters that have arrived on the uart
void ICACHE_FLASH_ATTR
level_msg_parse_buf(char *buf, short length) {
  // do LEVEL parsing
	if (length > 2) {
	    // proper message packet, invoke command processor after checking CRC
	    //os_printf("SLIP: rcv %d\n", slip_len);
		uint8_t msgLen = buf[0];
		if(msgLen<length)
		{
			uint16_t crc = crc_check((uint8_t*)&buf[1], msgLen);
			uint16_t rcv = ((uint16_t)buf[length-1]) | ((uint16_t)buf[length-2] << 8);
			if (crc == rcv) {
				msgParsePacket((char*)&buf[1], msgLen);
			} else {
				os_printf("SLIP: bad CRC, crc=%04x rcv=%04x len=%d\n", crc, rcv, length);

				for (short i=0; i<length; i++) {
					if (buf[i] >= ' ' && buf[i] <= '~') {
						DBG("%c", buf[i]);
					} else {
						DBG("\\%02X", buf[i]);
					}
				}
				DBG("\n");
			}
		}

	  }

}

void ICACHE_FLASH_ATTR
level_msg_recv(char *buf, short length)
{
	// append what we got to what we have accumulated
	if (responseLen + length < RESP_SZ-1) {
		char *rb = responseBuf+responseLen;

		os_memcpy(rb,buf,length);
		responseLen += length;
	}

	if(responseBuf[responseLen-1] == CMD_END)
	{
		DBG("Get end of msg!");
		//search for start of msg
		char *rb = responseBuf;
		bool msgValid = false;
		for(short i=0;i < responseLen ; i++)
		{
			if(responseBuf[i] == CMD_BEGIN)
			{
				rb = responseBuf+i+1;

				if(*rb < responseLen - i -2)
				{
					msgValid = true;
					DBG("Message valid \n");
					break;
				}
			}
		}
		if(msgValid)
		{
			uint8_t msgLength = *rb;
			msgParsePacket(rb,msgLength + 2);
			DBG("Let's parse msg!\n");
			DBG("Response Length = %u\n",msgLength + 2);
			for(short i=0;i<msgLength + 2;++i)
			{
				DBG("\\%02X", rb[i]);

			}
			level_msg_parse_buf(rb,msgLength + 3);
		}

	}


}

void ICACHE_FLASH_ATTR
msgRespBufferInit()
{
	responseLen = 0;
	os_memset(responseBuf,0,RESP_SZ);

}
void ICACHE_FLASH_ATTR
msgInit()
{
	msgRespBufferInit();
	//set a timer to check whether got ip from router succeed or not.
	os_timer_disarm(&acq_timer);
	os_timer_setfn(&acq_timer, (os_timer_func_t *)tank_acq_level, NULL);
	//Connect to server after reset 30'
	os_timer_arm(&acq_timer, flashConfig.data_interval * 1000, 1);

	uart_add_recv_cb(&msgUartCb);
	DBG("TANK message init \n");

}
// callback with a buffer of characters that have arrived on the uart
void ICACHE_FLASH_ATTR
msgUartCb(char *buf, short length)
{
	DBG("Tank message recv\n");
	 level_msg_recv(buf,length);

}

int ICACHE_FLASH_ATTR
mqttStatusMsg(char *buf) {

  // compose MQTT message
  return os_sprintf(buf,
    "{\"fuel_level\":%lu, \"water_level\":%lu, \"thermo\":%u}",
    tank_data.level[0],tank_data.level[1],tank_data.thermo);
}

// Timer callback to send an RSSI update to a monitoring system
static void ICACHE_FLASH_ATTR mqttStatusCb(void) {
  if (!flashConfig.mqtt_status_enable || os_strlen(flashConfig.mqtt_status_topic) == 0 ||
    mqttClient.connState != MQTT_CONNECTED)
  {
	  DBG("MQTT fail!\n");
    return;
  }
  char buf[128];
  mqttStatusMsg(buf);
  DBG("MQTT publish: %s\n",buf);
  MQTT_Publish(&mqttClient, flashConfig.mqtt_status_topic, buf, os_strlen(buf), 1, 0);
}
