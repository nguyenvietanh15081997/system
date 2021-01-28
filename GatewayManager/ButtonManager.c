#include "../GatewayManager/ButtonManager.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/OpCode.h"
#include "../GatewayManager/Light.h"



//example message to control light
uint8_t ONMES1[14]  = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x01, 0x00};
uint8_t OFFMES1[14] = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x00, 0x00};

remotersp * vrts_Remote_Rsp;

bool IsRemoteSetup(remotersp * rsp,unsigned char parButtonId,unsigned char parModeId,unsigned char parSenceId1,unsigned char parSenceId2)
{
	if(rsp->buttonID == parButtonId){
		if(rsp->modeID == parModeId){
			if(rsp->senceID[0] == parSenceId1 && rsp->senceID[1] == parSenceId2){
				return true;
			}
		}
	}
	else return false;
}
/*
 * Code old, without header
 */
//void SetSceneForRemote(uint16_t addressremote,uint8_t buttonId, uint8_t modeId, uint16_t sceneId)
//{
//	vrts_CMD_STRUCTURE.adr_dst[0]= addressremote & 0xFF;
//	vrts_CMD_STRUCTURE.adr_dst[1]= (addressremote>>8) & 0xFF;
//	vrts_CMD_STRUCTURE.opCode[0]= OPCODEREMOTERSP & 0xFF;
//	vrts_CMD_STRUCTURE.opCode[1]= (OPCODEREMOTERSP>>8) & 0xFF;
//	vrts_CMD_STRUCTURE.para[0]= buttonId;
//	vrts_CMD_STRUCTURE.para[1]= modeId;
//	vrts_CMD_STRUCTURE.para[2]= sceneId & 0xFF;
//	vrts_CMD_STRUCTURE.para[3]= (sceneId>>8) & 0xFF;
//}
void SetSceneForRemote(uint16_t addressremote,uint16_t header, uint8_t buttonId, uint8_t modeId, uint16_t sceneId, uint16_t appID, uint8_t SrgbID)
{
	vrts_CMD_STRUCTURE.adr_dst[0]= addressremote & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1]= (addressremote>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0]= OPCODEREMOTERSP & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1]= (OPCODEREMOTERSP>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0]= header & 0xFF;
	vrts_CMD_STRUCTURE.para[1]= (header>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2]= buttonId;
	vrts_CMD_STRUCTURE.para[3]= modeId;
	vrts_CMD_STRUCTURE.para[4]= sceneId & 0xFF;
	vrts_CMD_STRUCTURE.para[5]= (sceneId>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6]= appID & 0xFF;
	vrts_CMD_STRUCTURE.para[7]= (appID >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8]= SrgbID;
	int i;
	for(i=0; i<7; i++){
		vrts_CMD_STRUCTURE.para[i+9]= 0;
	}
}
