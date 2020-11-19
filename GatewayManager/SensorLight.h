#ifndef GATEWAYMANAGER_SENSORLIGHT_H_
#define GATEWAYMANAGER_SENSORLIGHT_H_

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

extern  uint16_t SENSOR_GATEWAY_RSP   ;

extern bool flag_sensor_rsp;
extern unsigned int lux;

float Sensor_Lux(uint16_t rsp_lux);
#ifdef __cplusplus
}
#endif

#endif
