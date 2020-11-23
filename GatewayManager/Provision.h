#ifndef GATEWAYMANAGER_PROVISION_H_
#define GATEWAYMANAGER_PROVISION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../GatewayManager/GateInterface.h"


extern pthread_t tmp ;

void ControlMessage(uint16_t lengthmessage,uint8_t *Message);
void *ProvisionThread (void *argv );

#ifdef __cplusplus
}
#endif

#endif
