#include "../GatewayManager/Light.h"
#include "../GatewayManager/GateInterface.h"
#include "../GatewayManager/Provision.h"


void ResetNode(uint16_t uniAddNode)
{
	vrts_MESSAGE_STRUCTURE->HCI_CMD_GATEWAY[0] = HCI_CMD_GATEWAY_CMD & 0xff;
	vrts_MESSAGE_STRUCTURE->HCI_CMD_GATEWAY[1] = (HCI_CMD_GATEWAY_CMD>>8) & 0xff;
	vrts_MESSAGE_STRUCTURE->retry_cnt = 0x02;
	//vrts_MESSAGE_STRUCTURE->rsp_max = 0x01;
	//vrts_MESSAGE_STRUCTURE->adr_dst[0] = uniAddNode[0];
	//vrts_MESSAGE_STRUCTURE->adr_dst[1] = uniAddNode[1];
	vrts_MESSAGE_STRUCTURE->OpCode[0] = NODE_RESET & 0xff;
	vrts_MESSAGE_STRUCTURE->OpCode[1]= (NODE_RESET >> 8) & 0xff;
}
//void
