#include "../GatewayManager/SensorLight.h"
#include "../GatewayManager/Provision.h"

bool flag_sensor_light_rsp = false;
uint16_t  Value_Lux = 0;

uint8_t ONMES[14]  = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x01, 0x00};
uint8_t OFFMES[14] = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x00, 0x00};

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
void Process_Lux()
{
	puts("ok");
	if(flag_sensor_light_rsp){
		flag_sensor_light_rsp = false;
		if(Value_Lux < 500){
			ControlMessage(14, ONMES);
			puts("On");
			sleep(2);
		}
		else {
			ControlMessage(14, OFFMES);
			puts("Off");
			sleep(2);
		}
	}
}
