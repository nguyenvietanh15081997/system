/*
 * ButtonManager.c
 */

#include "../GatewayManager/ButtonManager.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/OpCode.h"
#include "../GatewayManager/Light.h"

remotersp * vrts_Remote_Rsp;

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

/*
 * TODO: xây dựng một số byte quan trọng trong bản tin điều khiển set scene cho remote
 * - hàm này sẽ được gọi trong hàm FunctionPre() trong Light.h
 * - cần xem xét tối ưu lại
 */

