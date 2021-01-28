#include "../GatewayManager/Light.h"
#include "../GatewayManager/GateInterface.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/SensorLight.h"

cmdcontrol_t vrts_CMD_STRUCTURE;
uint8_t parRetry_cnt = 0x02;
uint8_t parRsp_Max = 0x01;
uint8_t parFuture = 0;

/*
 * TODO: functions process message send
 */
void ResetNode(uint16_t uniAdrReset)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrReset & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrReset>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = NODE_RESET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1]= (NODE_RESET >> 8) & 0xFF;
}
void Lightness_Get()
{
	vrts_CMD_STRUCTURE.adr_dst[0] = 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTNESS_GET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTNESS_GET>>8) & 0xFF;
}
/*
 * TODO: HOw many byte decide to dim, cct, hsl in data fields
 */
void Lightness_Set(uint16_t uniAdrSetDim, uint16_t valueLightness)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSetDim & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSetDim>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTNESS_SET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTNESS_SET>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = 0;
	vrts_CMD_STRUCTURE.para[1] = valueLightness & 0xFF;
}
void CCT_Set(uint16_t uniAdrSetCCT, uint16_t valueCCT)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSetCCT & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSetCCT>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHT_CTL_TEMP_SET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHT_CTL_TEMP_SET>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = valueCCT & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (valueCCT>>8) & 0xFF;
	char i;
	for (i=2;i<=4;i++)
	{
		vrts_CMD_STRUCTURE.para[i] = 0x00;
	}
}
void AddGroup(uint16_t uniAdrAddGroup,uint8_t adrGroup)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] =  CFG_MODEL_SUB_ADD & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (CFG_MODEL_SUB_ADD>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = adrGroup;
	vrts_CMD_STRUCTURE.para[3] = 0xC0;
	vrts_CMD_STRUCTURE.para[4] = 0x00;
	vrts_CMD_STRUCTURE.para[5] = 0x10;
}
void DelGroup(uint16_t uniAdrAddGroup,uint8_t adrGroup)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] =  CFG_MODEL_SUB_DEL & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (CFG_MODEL_SUB_DEL>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = adrGroup;
	vrts_CMD_STRUCTURE.para[3] = 0xC0;
	vrts_CMD_STRUCTURE.para[4] = 0x00;
	vrts_CMD_STRUCTURE.para[5] = 0x10;
}
void AddSence(uint16_t uniAdrSence, uint16_t senceID)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSence & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSence>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SCENE_STORE & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SCENE_STORE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = senceID & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (senceID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2]= vrts_CMD_STRUCTURE.para[3]= vrts_CMD_STRUCTURE.para[4]= 0;
}
void CallSence(uint16_t senceId)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SCENE_RECALL & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SCENE_RECALL>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = senceId & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (senceId>>8) & 0xFF;
}
void DelSence(uint16_t uniAdrDelSence, uint16_t senceId)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrDelSence & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrDelSence>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SCENE_DEL & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SCENE_DEL>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = senceId & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (senceId>>8) & 0xFF;
}
void ControlOnOff(uint16_t uniAdrControlOnOff,uint8_t statuOnOff)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrControlOnOff & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrControlOnOff>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] =  LIGHTOPCODE_ONOFF  & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTOPCODE_ONOFF >>8) & 0xFF;
	if(statuOnOff == 1){
		vrts_CMD_STRUCTURE.para[0] = statuOnOff;
	}
	if(statuOnOff == 0){
		vrts_CMD_STRUCTURE.para[0] = statuOnOff;
	}

	vrts_CMD_STRUCTURE.para[1] = 0;
}
/*
 * TODO: Update status of light
 */
void UpdateLight()
{
	vrts_CMD_STRUCTURE.adr_dst[0] = 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTOPCODE_UPDATE & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTOPCODE_UPDATE>>8)& 0xFF;
}

/*
 * TODO: Set timepoll for lightsensor
 */
void SetTimePoll(uint16_t uniAdrSensor, uint16_t timePoll)
{
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSensor & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSensor>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] =  SENSOR_DESCRIP_GET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SENSOR_DESCRIP_GET>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (timePoll>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = timePoll & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = parFuture;
}

/*
 * Ham ControlMessage de nhan chuoi can truyen
 * TODO: gan struct vao chuoi xong chuyen uart
 *
 */
//void SetSenceRemote(uint16_t uniAdrRemoteSetSence, uint16_t senceId)
//{
//	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrRemoteSetSence & 0xFF;
//	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrRemoteSetSence>>8) & 0xFF;
//	//vrts_CMD_STRUCTURE.opCode[0]
//}
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
				uint8_t cmdLenght)
{
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0] = cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1] = (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = 0;
	vrts_CMD_STRUCTURE.opCode00[1] = 0;
	vrts_CMD_STRUCTURE.opCode00[2] = 0;
	vrts_CMD_STRUCTURE.opCode00[3] = 0;
	vrts_CMD_STRUCTURE.retry_cnt = parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max = 0;
	if(Func == ResetNode_typedef){
		ResetNode(unicastAdr);
	}
	else if(Func == Lightness_Get_typedef){
		Lightness_Get();
	}
	else if(Func == AddGroup_typedef){
		AddGroup(unicastAdr, adrGroup);
	}
	else if(Func == DelGroup_typedef){
		DelGroup(unicastAdr, adrGroup);
	}
	else if(Func == ControlOnOff_typedef){
		ControlOnOff(unicastAdr,parStatusOnOff);
	}
	else if (Func == SetTimePoll_typedef){
		SetTimePoll(unicastAdr, parTimePoll);
	}
	else if (Func == CCT_Set_typedef)
	{
		CCT_Set(unicastAdr,parCCT);
	}
	else if (Func == Lightness_Set_typedef){
		Lightness_Set(unicastAdr, parLightness);
	}
	else if (Func == UpdateLight_typedef){
		UpdateLight();
	}
	else if(Func == AddSence_typedef){
		AddSence(unicastAdr, parSenceId);
	}
	else if(Func == DelSence_typedef){
		DelSence(unicastAdr, parSenceId);
	}
	else if(Func == CallSence_typedef){
		CallSence(parSenceId);
	}
	uint8_t *TempData;

	TempData = (uint8_t *)&vrts_CMD_STRUCTURE;
	int i;
	    printf("Content:");
	    for (i = 0; i <cmdLenght; i++){
	    	printf("%2x-",TempData[i]);
	    }
	    printf("\n");
	    ControlMessage(cmdLenght, TempData);
}


void StoreSceneRemote(uint16_t cmd, uint16_t adrRemote, uint8_t header, uint8_t buttonID, uint8_t modeID, uint16_t sceneID,uint16_t appID, uint8_t SrgbID,uint8_t cmdLength)
{
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0] = cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1] = (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = 0;
	vrts_CMD_STRUCTURE.opCode00[1] = 0;
	vrts_CMD_STRUCTURE.opCode00[2] = 0;
	vrts_CMD_STRUCTURE.opCode00[3] = 0;
	vrts_CMD_STRUCTURE.retry_cnt = parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max = 0;
	SetSceneForRemote(adrRemote, header, buttonID, modeID, sceneID, appID, SrgbID);

	uint8_t *TempData;
	TempData = (uint8_t *)&vrts_CMD_STRUCTURE;
	int i;
		printf("Content:");
		for (i = 0; i <cmdLength; i++){
			printf("%2x-",TempData[i]);
		}
		printf("\n");
		ControlMessage(cmdLength, TempData);
}
void StoreSceneSensor(uint16_t cmd, uint16_t adrSensor, uint8_t header, uint8_t stt, uint16_t condition, uint16_t low_lux,\
		uint16_t hight_lux, uint16_t action, uint16_t sceneID, uint16_t appID, uint8_t srgbID, uint16_t cmdLength)
{
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0] = cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1] = (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = 0;
	vrts_CMD_STRUCTURE.opCode00[1] = 0;
	vrts_CMD_STRUCTURE.opCode00[2] = 0;
	vrts_CMD_STRUCTURE.opCode00[3] = 0;
	vrts_CMD_STRUCTURE.retry_cnt = parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max = 0;
	SetScenceForSensor(adrSensor, header, stt, condition, low_lux, hight_lux, action, sceneID, appID, srgbID);

	uint8_t *TempData;
	TempData = (uint8_t *)&vrts_CMD_STRUCTURE;
	int i;
		printf("Content:");
		for (i = 0; i <cmdLength; i++){
			printf("%2x-",TempData[i]);
		}
		printf("\n");
		ControlMessage(cmdLength, TempData);
}




