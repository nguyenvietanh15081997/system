#include "../GatewayManager/Battery.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"

batteryRsp * vrts_Battery_Rsp;
static uint16_t valueBattery;

void ProcessBat(batteryRsp * batRsp)
{
	valueBattery = batRsp->batValue[1] | (batRsp->batValue[0]<<8);
	printf ("Battery= %d %\n",valueBattery);
}
