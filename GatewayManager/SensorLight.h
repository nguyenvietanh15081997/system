/*
 * SensorLight.h manage tasks-related sensor( light, PIR)
 *
 */
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

/*Frame of rsp data sensor*/
typedef struct lightsensorRsp
{
	uint8_t  typeDev[2];
	uint8_t  luxValue[2];
	uint8_t  future[4];
}lightsensorRsp;
extern lightsensorRsp * vrts_LighSensor_Rsp;

/*Define friend_poll*/
#define SENSOR_DESCRIP_GET     0x3082
#define SENSOR_DESCRIP_STATUS  0x51

extern uint16_t  value_Lux;

/*
 * Calculate lux of lightsensor
 *
 * @param rsp_lux two byte lux
 * @return lux value calculated
 */
unsigned int CalculateLux(unsigned int rsp_lux);

/*
 * Process when have message of sensor
 *
 * @param rsp message of sensor
 * @return null
 */
void ProcessLightSensor(lightsensorRsp *rsp);

/*
 * Build message set scene for sensor
 *TODO: to review
 */
void SetScenceForSensor(uint16_t addressSensor, uint16_t header, uint8_t stt, uint16_t condition, uint16_t low_lux, uint16_t hight_lux, \
		uint16_t action, uint16_t sceneID, uint16_t appID, uint8_t srgbID);
#ifdef __cplusplus
}
#endif
#endif
