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

typedef struct pirsensorRsp
{
	uint8_t  typeDev[2];
	uint8_t  pir[2];
	uint8_t  future[4];
}pirsensorRsp;
extern pirsensorRsp * vrts_PirSensor_Rsp;

typedef struct pmsensorRsp_Temp_Hum
{
	uint8_t header[2];
	uint8_t hum[2];
	uint8_t temp[4];
}pmsensorRsp_Temp_Hum;
extern pmsensorRsp_Temp_Hum * vrts_PMSensor_Rsp_Temp_Hum;

typedef struct pmsensorRsp_PM
{
	uint8_t header[2];
	uint8_t PM_2_5[2];
	uint8_t PM_10[2];
	uint8_t PM_1_0[2];
}pmsensorRsp_PM;
extern pmsensorRsp_PM * vrts_PMSensor_Rsp_PM;

typedef struct{
	union{
		uint32_t data;
		struct{
			uint32_t store				:8;//8 bit not use
			uint32_t Lux_hi				:10;//10 bit lux hi
			uint32_t Lux_low			:10;//10 bit lux low
			uint32_t Light_Conditon		:3; // 7 bit low
			uint32_t Pir_Conditon		:1; // 1 bit hight
		};
	};
}RD_Sensor_data_tdef;


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

#ifdef __cplusplus
}
#endif
#endif
