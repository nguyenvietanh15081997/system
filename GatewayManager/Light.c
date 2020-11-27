#include "../GatewayManager/Light.h"
#include "../GatewayManager/GateInterface.h"
#include "../GatewayManager/Provision.h"

cmdcontrol_t *vrts_CMD_STRUCTURE;
uint8_t parRetry_cnt = 0x02;
uint8_t parRsp_Max = 0x01;

void ResetNode(uint16_t uniAdrReset)
{
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[0] = HCI_CMD_GATEWAY_CMD & 0xFF;
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[1] = (HCI_CMD_GATEWAY_CMD>>8) & 0xFF;
//	vrts_CMD_STRUCTURE->retry_cnt = parRetry_cnt;
//	vrts_CMD_STRUCTURE->rsp_max = parRsp_Max;
	vrts_CMD_STRUCTURE->adr_dst[0] = uniAdrReset & 0xFF;
	vrts_CMD_STRUCTURE->adr_dst[1] = (uniAdrReset>>8) & 0xFF;
	vrts_CMD_STRUCTURE->opCode[0] = NODE_RESET & 0xFF;
	vrts_CMD_STRUCTURE->opCode[1]= (NODE_RESET >> 8) & 0xFF;
}
void Lightess_Get()
{
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[0] = HCI_CMD_GATEWAY_CMD & 0xFF;
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[1] = (HCI_CMD_GATEWAY_CMD>>8) & 0xFF;
//	vrts_CMD_STRUCTURE->retry_cnt = parRetry_cnt;
//	vrts_CMD_STRUCTURE->rsp_max = parRsp_Max;
	vrts_CMD_STRUCTURE->adr_dst[0] = 0xFF;
	vrts_CMD_STRUCTURE->adr_dst[1] = 0xFF;
	vrts_CMD_STRUCTURE->opCode[0] = LIGHTNESS_GET & 0xFF;
	vrts_CMD_STRUCTURE->opCode[1] = (LIGHTNESS_GET>>8) & 0xFF;
}
void AddGroup(uint16_t uniAdrAddGroup,uint8_t adrGroup)
{
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[0] = HCI_CMD_GATEWAY_CMD & 0xFF;
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[1] = (HCI_CMD_GATEWAY_CMD>>8) & 0xFF;
//	vrts_CMD_STRUCTURE->retry_cnt = parRetry_cnt;
//	vrts_CMD_STRUCTURE->rsp_max = parRsp_Max;
	vrts_CMD_STRUCTURE->adr_dst[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE->adr_dst[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE->opCode[0] =  CFG_MODEL_SUB_ADD & 0xFF;
	vrts_CMD_STRUCTURE->opCode[1] = (CFG_MODEL_SUB_ADD>>8) & 0xFF;
	vrts_CMD_STRUCTURE->para[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE->para[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE->para[2] = adrGroup;
	vrts_CMD_STRUCTURE->para[3] = 0xC0;
	vrts_CMD_STRUCTURE->para[4] = 0x00;
	vrts_CMD_STRUCTURE->para[5] = 0x10;
}
void DelGroup(uint16_t uniAdrAddGroup,uint8_t adrGroup)
{
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[0] = HCI_CMD_GATEWAY_CMD & 0xFF;
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[1] = (HCI_CMD_GATEWAY_CMD>>8) & 0xFF;
//	vrts_CMD_STRUCTURE->retry_cnt = parRetry_cnt;
//	vrts_CMD_STRUCTURE->rsp_max = parRsp_Max;
	vrts_CMD_STRUCTURE->adr_dst[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE->adr_dst[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE->opCode[0] =  CFG_MODEL_SUB_DEL & 0xFF;
	vrts_CMD_STRUCTURE->opCode[1] = (CFG_MODEL_SUB_DEL>>8) & 0xFF;
	vrts_CMD_STRUCTURE->para[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE->para[1] = (uniAdrAddGroup>>8) & 0xFF;
	vrts_CMD_STRUCTURE->para[2] = adrGroup;
	vrts_CMD_STRUCTURE->para[3] = 0xC0;
	vrts_CMD_STRUCTURE->para[4] = 0x00;
	vrts_CMD_STRUCTURE->para[5] = 0x10;
}
void ControlOnOff(uint8_t uniAdrControlOnOff,uint8_t statuOnOff)
{
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[0] = HCI_CMD_GATEWAY_CMD & 0xFF;
//	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[1] = (HCI_CMD_GATEWAY_CMD>>8) & 0xFF;
//	vrts_CMD_STRUCTURE->retry_cnt = parRetry_cnt;
//	vrts_CMD_STRUCTURE->rsp_max = parRsp_Max;
	vrts_CMD_STRUCTURE->adr_dst[0] = uniAdrControlOnOff & 0xFF;
	vrts_CMD_STRUCTURE->adr_dst[1] = (uniAdrControlOnOff>>8) & 0xFF;
	vrts_CMD_STRUCTURE->opCode[0] =  LIGHTOPCODE_ONOFF  & 0xFF;
	vrts_CMD_STRUCTURE->opCode[1] = (LIGHTOPCODE_ONOFF >>8) & 0xFF;
	if(statuOnOff == 1){
		vrts_CMD_STRUCTURE->para[0] = statuOnOff;
	}
	if(statuOnOff == 0){
		vrts_CMD_STRUCTURE->para[0] = statuOnOff;
	}

	vrts_CMD_STRUCTURE->para[1] = 0;
}
void FunctionPre(uint16_t cmd,functionTypeDef Fun, uint8_t dodaibantin,uint16_t unicastAdr,uint8_t adrGroup, uint8_t parStatusOnOff)
{
	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[0] = cmd & 0xFF;
	vrts_CMD_STRUCTURE->HCI_CMD_GATEWAY[1] = (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE->opCode00[0] = 0;
	vrts_CMD_STRUCTURE->opCode00[1] = 0;
	vrts_CMD_STRUCTURE->opCode00[2] = 0;
	vrts_CMD_STRUCTURE->opCode00[3] = 0;
	vrts_CMD_STRUCTURE->retry_cnt = parRetry_cnt;
	vrts_CMD_STRUCTURE->rsp_max = parRsp_Max;
	if(Fun == ResetNode_typedef){
		ResetNode(unicastAdr);
	}
	else if(Fun == Lightess_Get_typedef){
		Lightess_Get();
	}
	else if(Fun == AddGroup_typedef){
		AddGroup(unicastAdr, adrGroup);
	}
	else if(Fun == DelGroup_typedef){
		DelGroup(unicastAdr, adrGroup);
	}
	else if(Fun == ControlOnOff_typedef)
	{
		ControlOnOff(unicastAdr,parStatusOnOff);
	}
	uint8_t * CMDControl =  (&vrts_CMD_STRUCTURE->para[0]);
	//printf ("%d",(*CMDControl));
	ControlMessage(dodaibantin, *CMDControl);
}






