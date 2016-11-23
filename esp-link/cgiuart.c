/*
Cgi/template routines for the /wifi url.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return.
 * ----------------------------------------------------------------------------
 * Heavily modified and enhanced by Thorsten von Eicken in 2015
 * ----------------------------------------------------------------------------
 */

#include <esp8266.h>
#include "cgiwifi.h"
#include "cgi.h"
#include "config.h"

#ifdef CGIWIFI_DBG
#define DBG(format, ...) do { os_printf(format, ## __VA_ARGS__); } while(0)
#else
#define DBG(format, ...) do { } while(0)
#endif

# define VERS_STR_STR(V) #V
# define VERS_STR(V) VERS_STR_STR(V)


//static bool ICACHE_FLASH_ATTR parse_ip(char *buff, ip_addr_t *ip_ptr) {
//  char *next = buff; // where to start parsing next integer
//  int found = 0;     // number of integers parsed
//  uint32_t ip = 0;   // the ip addres parsed
//  for (int i=0; i<32; i++) { // 32 is just a safety limit
//    char c = buff[i];
//    if (c == '.' || c == 0) {
//      // parse the preceding integer and accumulate into IP address
//      bool last = c == 0;
//      buff[i] = 0;
//      uint32_t v = atoi(next);
//      ip = ip | ((v&0xff)<<(found*8));
//      next = buff+i+1; // next integer starts after the '.'
//      found++;
//      if (last) { // if at end of string we better got 4 integers
//        ip_ptr->addr = ip;
//        return found == 4;
//      }
//      continue;
//    }
//    if (c < '0' || c > '9') return false;
//  }
//  return false;
//}


// Cgi to return various Uart information
int ICACHE_FLASH_ATTR cgiUartInfo(HttpdConnData *connData) {
  char buff[1024];

  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.


  int len = os_sprintf(buff, "{\"baud\":\"%ld\"",flashConfig.baud_rate);
  len += os_sprintf(buff+len, ",\"databits\":\"%d\"",flashConfig.data_bits);
  len += os_sprintf(buff+len, ",\"parity\":\"%d\"",flashConfig.parity);
  len += os_sprintf(buff+len, ",\"stopbits\":\"%d\"}",flashConfig.stop_bits);

  DBG("%s",buff);

  jsonHeader(connData, 200);
  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiUartConfig(HttpdConnData *connData) {
  char baudrate[16];
  char databits[8];
  char parity[8];
  char stopbit[8];

  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.

  int bl=httpdFindArg(connData->getArgs, "baud", baudrate, sizeof(baudrate));
  int dl=httpdFindArg(connData->getArgs, "databits", databits, sizeof(databits));
  int pl=httpdFindArg(connData->getArgs, "parity", parity, sizeof(parity));
  int sl=httpdFindArg(connData->getArgs, "stop", stopbit, sizeof(stopbit));

  if (!(dl > 0 && sl >= 0 && pl >= 0 && bl >= 0)) {
      jsonHeader(connData, 400);
      httpdSend(connData, "Request is missing fields", -1);
      return HTTPD_CGI_DONE;
    }
  int temp = atoi(baudrate);
  if (temp >= 1200 && temp <= 1000000) {
	flashConfig.baud_rate = temp;
  }
  temp = atoi(databits);
  flashConfig.data_bits = temp;
  temp = atoi(parity);
  flashConfig.parity = temp;
  temp = atoi(stopbit);
  flashConfig.stop_bits = temp;

  if (configSave()) {
      httpdStartResponse(connData, 204);
      httpdEndHeaders(connData);
    }
    else {
      httpdStartResponse(connData, 500);
      httpdEndHeaders(connData);
      httpdSend(connData, "Failed to save config", -1);
    }

  return HTTPD_CGI_DONE;
}

// Cgi to return various sensor information
int ICACHE_FLASH_ATTR cgiSensorInfo(HttpdConnData *connData) {
  char buff[1024];

  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.

  int len;
  len = os_sprintf(buff, "{\"id\": \"%02X%02X%02X\"}",
		  flashConfig.device_id[0],flashConfig.device_id[1],flashConfig.device_id[2]);


  DBG("%s",buff);

  jsonHeader(connData, 200);
  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR cgiSensorConfig(HttpdConnData *connData) {
  char sId[16];

  if (connData->conn==NULL) return HTTPD_CGI_DONE; // Connection aborted. Clean up.

  int il=httpdFindArg(connData->getArgs, "sid", sId, sizeof(sId));

  bool valid = true;

  for(uint8_t i = 0; i < il; ++i)
  {
	  if(!isxdigit((int)sId[i]) )
	  {
		  valid = false;
		  break;
	  }
  }
  DBG("il: %u",il);
  if(valid) DBG("valid: true");
  else DBG("valid: false");
  if (!( (il == 6) && valid) ) {
      jsonHeader(connData, 400);
      httpdSend(connData, "Request is missing fields", -1);
      return HTTPD_CGI_DONE;
    }
  // parse static ID params

  char sTemp[9] = {"0x000000"};

  for(uint8_t i=2;i<8;++i)
  {
	  sTemp[i]=sId[i-2];
  }
      long temp ;

      char *endp;
      temp = strtoul(sTemp, NULL, 16);
      DBG("temp: %lX",temp);
      // save the params in flash
  flashConfig.device_id[0] = (uint8_t) (temp>>16);
  flashConfig.device_id[1] = (uint8_t) (temp>>8);
  flashConfig.device_id[2] = (uint8_t) (temp);


  if (configSave()) {
      httpdStartResponse(connData, 204);
      httpdEndHeaders(connData);
    }
    else {
      httpdStartResponse(connData, 500);
      httpdEndHeaders(connData);
      httpdSend(connData, "Failed to save config", -1);
    }

  return HTTPD_CGI_DONE;
}

