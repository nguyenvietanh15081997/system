#include "../GatewayManager/SensorLight.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"

lightsensorRsp * vrts_LighSensor_Rsp;
//bool flag_sensor_light_rsp = false;
uint16_t  value_Lux = 0;
static uint16_t luxReg;

uint8_t ONMES[14]  = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x01, 0x00};
uint8_t OFFMES[14] = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x00, 0x00};

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
}
void SetScenceForSensor(uint16_t addressSensor, uint16_t header, uint8_t stt, uint16_t condition, uint16_t low_lux, uint16_t hight_lux, \
		uint16_t action, uint16_t sceneID, uint16_t appID, uint8_t srgbID)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = addressSensor & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (addressSensor>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SCENE_STORE & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SCENE_STORE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = header & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (header>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = stt;
	vrts_CMD_STRUCTURE.para[3] = (condition>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = condition & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = low_lux & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (low_lux>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = hight_lux & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (hight_lux>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = action & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = (action>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[11] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[12] = (sceneID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[13] = appID & 0xFF;
	vrts_CMD_STRUCTURE.para[14] = (appID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[15] = srgbID;
}
