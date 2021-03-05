/*
 * Battery.c
 */

#include "../GatewayManager/Battery.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/slog.h"
batteryRsp * vrts_Battery_Rsp;
uint16_t valueBattery;

uint16_t ProcessBat(batteryRsp * batRsp)
{
	valueBattery = batRsp->batValue[1] | (batRsp->batValue[0]<<8);
	slog_trace("Battery: %d %",valueBattery);
	return (valueBattery);
}
