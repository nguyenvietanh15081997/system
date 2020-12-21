#ifndef GATEWAYMANAGER_LIGHT_H_
#define GATEWAYMANAGER_LIGHT_H_


#ifdef __cplusplus
extern "C" {
#endif
#include "../GatewayManager/OpCode.h"


typedef struct{
	uint8_t HCI_CMD_GATEWAY[2];// CMD
	uint8_t opCode00[4]; // 00 00 00 00
	uint8_t retry_cnt; // num of send
	uint8_t rsp_max;  // num of rsp
	uint8_t adr_dst[2];    // adr
	uint8_t opCode[2];
	uint8_t para[16];
} cmdcontrol_t;

extern cmdcontrol_t vrts_CMD_STRUCTURE;
#define NULL8    0x00
#define NULL16   0x0000

#define LIGHTOPCODE_UPDATE      0x0182
#define LIGHTOPCODE_SELECT      0x6d82
#define LIGHTOPCODE_ONOFF       0x0282
#define LIGHTOPCODE_DIM         0x4d82
#define LIGHTOPCODE_SENCE_SET   0x4382
#define LIGHTOPCODE_SENCE_GET   0x4382
#define LIGHTOPCODE_SENCE_DEL   0x9e82

//status
#define LIGHTOPCODE_STATUS_ON   0x01
#define LIGHTOPCODE_STATUS_OFF  0x00
//
#define LIGHTNESS_GET		        	0x4B82
#define LIGHTNESS_SET		        	0x4C82
#define LIGHTNESS_SET_NOACK				0x4D82
#define LIGHTNESS_STATUS		    	0x4E82
#define LIGHTNESS_LINEAR_GET		    0x4F82
#define LIGHTNESS_LINEAR_SET		    0x5082
#define LIGHTNESS_LINEAR_SET_NOACK		0x5182
#define LIGHTNESS_LINEAR_STATUS		    0x5282
#define LIGHTNESS_LAST_GET		    	0x5382
#define LIGHTNESS_LAST_STATUS		    0x5482
#define LIGHTNESS_DEFULT_GET		    0x5582
#define LIGHTNESS_DEFULT_STATUS		    0x5682
#define LIGHTNESS_RANGE_GET		    	0x5782
#define LIGHTNESS_RANGE_STATUS		    0x5882
#define LIGHTNESS_DEFULT_SET		    0x5982
#define LIGHTNESS_DEFULT_SET_NOACK		0x5A82
#define LIGHTNESS_RANGE_SET		    	0x5B82
#define LIGHTNESS_RANGE_SET_NOACK		0x5C82
#define LIGHT_CTL_GET		        	0x5D82
#define LIGHT_CTL_SET		        	0x5E82
#define LIGHT_CTL_SET_NOACK				0x5F82
#define LIGHT_CTL_STATUS		    	0x6082
#define LIGHT_CTL_TEMP_GET		        0x6182
#define LIGHT_CTL_TEMP_RANGE_GET		0x6282
#define LIGHT_CTL_TEMP_RANGE_STATUS		0x6382
#define LIGHT_CTL_TEMP_SET				0x6482
#define LIGHT_CTL_TEMP_SET_NOACK		0x6582
#define LIGHT_CTL_TEMP_STATUS			0x6682
#define LIGHT_CTL_DEFULT_GET			0x6782
#define LIGHT_CTL_DEFULT_STATUS		    0x6882
#define LIGHT_CTL_DEFULT_SET			0x6982
#define LIGHT_CTL_DEFULT_SET_NOACK		0x6A82
#define LIGHT_CTL_TEMP_RANGE_SET		0x6B82
#define LIGHT_CTL_TEMP_RANGE_SET_NOACK	0x6C82

#define LIGHT_HSL_STATUS                0x7882
//
typedef enum{
	null = 0x00,
	ResetNode_typedef = 0x01,
	Lightness_Get_typedef = 0x02,
	AddGroup_typedef = 0x03,
	DelGroup_typedef = 0x04,
	ControlOnOff_typedef = 0x05,
	SetTimePoll_typedef = 0x06,
	UpdateLight_typedef = 0x07,
	CCT_Set_typedef = 0x08,
	Lightness_Set_typedef = 0x09a,
	AddSence_typedef = 0x0a,
	DelSence_typedef = 0x0b,
	CallSence_typedef = 0x0c
} functionTypeDef;

void ResetNode(uint16_t uniAdrReset);
void Lightness_Get();
void Lightness_Set(uint16_t uniAdrSetDim, uint16_t valueLightness);
void CCT_Set(uint16_t uniAdrSetCCT, uint16_t valueCCT);
void AddGroup(uint16_t uniAdrAddGroup,uint8_t adrGroup);
void DelGroup(uint16_t uniAdrAddGroup,uint8_t adrGroup);

void AddSence(uint16_t uniAdrSence, uint16_t senceID);
void CallSence(uint16_t senceId);
void DelSence(uint16_t uniAdrDelSence, uint16_t senceId);

void ControlOnOff(uint16_t uniAdrControlOnOff,uint8_t statuOnOff);
void UpdateLight();
void SetTimePoll(uint16_t uniAdrSensor, uint16_t timePoll);
void FunctionPer(uint16_t cmd,\
				functionTypeDef Func,\
				uint16_t unicastAdr,\
				uint8_t adrGroup,\
				uint8_t parStatusOnOff,\
				uint16_t parLightness,\
				uint16_t parCCT,\
				uint16_t parSenceId,\
				uint16_t parTimePoll,\
				uint16_t parH,
				uint16_t parS,
				uint16_t parL,
				uint8_t cmdLenght);
#ifdef __cplusplus
}
#endif

#endif 
