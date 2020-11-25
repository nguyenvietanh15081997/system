#ifndef GATEWAYMANAGER_PROVISION_H_
#define GATEWAYMANAGER_PROVISION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../GatewayManager/RingBuffer.h"
#include "../GatewayManager/ShareMessage.h"
#include "../GatewayManager/OpCode.h"


extern bool flag_selectmac;
extern bool flag_getpro_info;
extern bool flag_getpro_element;
extern bool flag_provision;
extern bool flag_mac;
extern bool flag_check_select_mac;
extern bool flag_done;


// timeout check data in buffer
extern unsigned int Timeout_CheckDataBuffer;


extern pthread_t tmp ;

void ControlMessage(uint16_t lengthmessage,uint8_t *Message);
void *ProvisionThread (void *argv );

#ifdef __cplusplus
}
#endif

#endif
