#include "../GatewayManager/SensorLight.h"

bool flag_sensor_rsp = false;

// use function when config transmit register value lux
float Sensor_Lux(uint16_t rsp_lux)
{
	uint16_t Lux_MSB;
	uint8_t Lux_LSB;
	float Lux_Value;
	Lux_MSB = rsp_lux | 0x0fff;
	Lux_LSB = (rsp_lux>>12) | 0x0f;
	Lux_Value = 0.01 * pow(2,Lux_LSB) * Lux_MSB;
	return Lux_Value;
}
