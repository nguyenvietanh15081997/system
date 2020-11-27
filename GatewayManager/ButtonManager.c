#include "../GatewayManager/ButtonManager.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/OpCode.h"



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


