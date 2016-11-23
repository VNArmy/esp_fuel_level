/*
 * levelmeasure.h
 *
 *  Created on: Jul 26, 2016
 *      Author: vnarmy
 */

#ifndef LEVELMEASURE_H_
#define LEVELMEASURE_H_

// Descriptor for a connection with message assembly storage
typedef struct level_data {
  uint8_t device_id[3];	  // id of assembled message and memo to calculate next message id
  uint32_t level[2];
  uint16_t thermo;
} level_data_t;
level_data_t tank_data;
uint8_t msgCreate(char* buffer, uint16_t msgtype);
void msgRespBufferInit();
void msgUartCb(char *buf, short length);
void msgInit();
#endif /* LEVELMEASURE_H_ */
