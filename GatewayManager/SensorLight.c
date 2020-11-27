#include "../GatewayManager/SensorLight.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"

lightsensorRsp * vrts_LighSensor_Rsp;
//bool flag_sensor_light_rsp = false;
uint16_t  value_Lux = 0;
static uint16_t luxReg;

uint8_t ONMES[14]  = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x01, 0x00};
uint8_t OFFMES[14] = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x00, 0x00};

/*
 * use function when config transmit register value lux
 * TODO: fix error don't use pow in this function
 */
unsigned int CalculateLux(unsigned int rsp_lux)
{
	unsigned int lux_LSB = 0;
	unsigned char lux_MSB = 0;
	unsigned int lux_Value = 0;
	unsigned int pow = 1;
	unsigned char i;
	lux_LSB = rsp_lux & 0x0FFF;
	lux_MSB = ((rsp_lux>>12) & 0x0F);
	//Lux_Value = 0.01 * pow(2,Lux_MSB) * Lux_LSB; //don't use
	for(i=0;i<lux_MSB;i++){
		pow=pow*2;
	}
	lux_Value=0.01 * pow * lux_LSB;
	return lux_Value;
}
void ProcessLightSensor(lightsensorRsp *rsp)
{
	luxReg=rsp->luxValue[1] | (rsp->luxValue[0]<<8);
	value_Lux = CalculateLux(luxReg);
	printf ("Lux= %d\n",value_Lux);
	if(value_Lux<500){
		FunctionPre(LIGHTOPCODE_ONOFF,ControlOnOff_typedef, 14,0xffff,0, 1);
	}
	else {
		FunctionPre(LIGHTOPCODE_ONOFF,ControlOnOff_typedef, 14,0xffff,0, 0);
	}
}
