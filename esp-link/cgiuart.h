#ifndef CGIUART_H
#define CGIUART_H

#include "httpd.h"

int cgiUartInfo(HttpdConnData *connData);
int cgiUartConfig(HttpdConnData *connData);
int cgiSensorConfig(HttpdConnData *connData) ;
int cgiSensorInfo(HttpdConnData *connData);
#endif
