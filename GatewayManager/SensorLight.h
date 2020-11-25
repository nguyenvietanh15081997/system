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

#define SENSOR_GATEWAY_RSP 0x0152


extern bool flag_sensor_light_rsp;
extern uint16_t  Value_Lux;

float Sensor_Lux(uint16_t rsp_lux);
void Process_Lux();
#ifdef __cplusplus
}
#endif

#endif
