/*
 * ButtonManager.c
 */

#include "../GatewayManager/ButtonManager.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/OpCode.h"
#include "../GatewayManager/Light.h"

remotersp * vrts_Remote_Rsp;
screenTouch *vrts_ScreenT_Rsp;

bool IsRemoteSetup(remotersp * rsp,unsigned char parButtonId,unsigned char parModeId)
{
	if(rsp->buttonID == parButtonId)
	{
		if(rsp->modeID == parModeId)
		{
				return true;
		}
	}
	else return false;
}

