#ifndef GATEWAYMANAGER_PROVISION_H_
#define GATEWAYMANAGER_PROVISION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../GatewayManager/RingBuffer.h"
#include "../GatewayManager/ShareMessage.h"
#include "../GatewayManager/OpCode.h"

/*
 * message control provision
 */
//extern uint8_t OUTMESSAGE_ScanStop[3]     = {0xE9, 0xFF, 0x01};
//extern uint8_t OUTMESSAGE_ScanStart[3]    = {0xE9, 0xFF, 0x00};
//extern uint8_t OUTMESSAGE_MACSelect[9]    = {0xE9, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//extern uint8_t OUTMESSAGE_GetPro[3] 	  = {0xE9, 0xFF, 0x0C};
//extern uint8_t OUTMESSAGE_Provision[28]   = {0};
//extern uint8_t OUTMESSAGE_BindingALl[22]  = {0xe9, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};

extern bool flag_selectmac;
extern bool flag_getpro_info;
extern bool flag_getpro_element;
extern bool flag_provision;
extern bool flag_mac;
extern bool flag_check_select_mac;
extern bool flag_done;
extern bool flag_setpro;
extern bool flag_admitpro;
extern bool flag_checkadmitpro;


// timeout check data in buffer
extern unsigned int Timeout_CheckDataBuffer;
extern unsigned char scanNotFoundDev;


extern pthread_t tmp ;

void ControlMessage(uint16_t lengthmessage,uint8_t *Message);
void *ProvisionThread (void *argv );

#ifdef __cplusplus
}
#endif

#endif
