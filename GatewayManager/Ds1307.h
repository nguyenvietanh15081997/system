#ifndef GATEWAYMANAGER_DS1307_H_
#define GATEWAYMANAGER_DS1307_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <endian.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mraa/i2c.h"


extern uint8_t dataDecTimeDs1307[7];
extern uint8_t dataTimeInternet[7];

uint8_t  BCD2DEC(uint8_t dataBCD);
uint8_t DEC2BCD(uint8_t dataDEC);
void Ds1307_Init();
void TimeForDs1307();
void TimeForInternet();
void SetTimeForDs1307(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t dates, uint8_t months, uint8_t years);
void * Time_Thread(void *argv);

#ifdef __cplusplus
}
#endif

#endif
