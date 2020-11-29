#ifndef GATEWAYMANAGER_BATTERY_H_
#define GATEWAYMANAGER_BATTERY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

typedef struct batteryRsp 
{
	uint8_t  typeDev[2];
	uint8_t  valueBat[2];
	uint8_t  future[4];
} batteryRsp;

uint16_t value_Bat = 0;

void ProcessBat();
#ifdef __cplusplus
}
#endif

#endif