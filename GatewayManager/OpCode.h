#ifndef GATEWAYMANAGER_OPCODE_H_
#define GATEWAYMANAGER_OPCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../GatewayManager/ShareMessage.h"

//mode


// rsp cmd part 
#define HCI_GATEWAY_RSP_UNICAST	   		0x80
#define HCI_GATEWAY_RSP_OP_CODE	 	 	0X81
#define HCI_GATEWAY_KEY_BIND_RSP 		0x82
#define HCI_GATEWAY_CMD_STATIC_OOB_RSP 	0x87  // HCI send back the static oob information 
#define HCI_GATEWAY_CMD_UPDATE_MAC	 	0x88
#define HCI_GATEWAY_CMD_PROVISION_EVT 	0x89
#define HCI_GATEWAY_CMD_KEY_BIND_EVT 	0x8a
#define HCI_GATEWAY_CMD_PRO_STS_RSP 	0x8b
#define HCI_GATEWAY_CMD_SEND_ELE_CNT 	0x8c
#define HCI_GATEWAY_CMD_SEND_NODE_INFO 	0x8d


// send cmd part 
#define HCI_GATEWAY_CMD_START				0x00
#define HCI_GATEWAY_CMD_STOP				0x01
#define HCI_GATEWAY_CMD_RESET       		0x02
#define HCI_GATEWAY_CMD_CLEAR_NODE_INFO		0x06
#define HCI_GATEWAY_CMD_SET_ADV_FILTER 		0x08
#define HCI_GATEWAY_CMD_SET_PRO_PARA 		0x09
#define HCI_GATEWAY_CMD_SET_NODE_PARA		0x0a
#define HCI_GATEWAY_CMD_START_KEYBIND 		0x0b
#define HCI_GATEWAY_CMD_GET_PRO_SELF_STS 	0x0c
#define HCI_GATEWAY_CMD_SET_DEV_KEY     	0x0d
#define HCI_GATEWAY_CMD_GET_SNO         	0x0e
#define HCI_GATEWAY_CMD_SET_SNO         	0x0f
#define HCI_GATEWAY_CMD_GET_UUID_MAC        0x10
#define HCI_GATEWAY_CMD_DEL_VC_NODE_INFO    0x11
#define HCI_GATEWAY_CMD_SEND_VC_NODE_INFO	0x12

//TSCRIPT
#define TSCRIPT_MESH_RX 			0x80
#define TSCRIPT_MESH_RX_NW 			0x90
#define TSCRIPT_GATEWAY_DIR_RSP 	0x91
#define HCI_GATEWAY_CMD_SAR_MSG		0x92
#define TSCRIPT_CMD_VC_DEBUG 		0xfa


//flag provision
#define HCI_GATEWAY_CMD_PROVISION_SUSCESS 0x01
#define HCI_GATEWAY_CMD_BIND_SUSCESS      0x01
#define HCI_GATEWAY_CMD_BIND_DONE         0x00

// user
#define HCI_CMD_USER_START			0xFF00
#define HCI_CMD_USER				HCI_CMD_USER_START
#define	HCI_CMD_USER_END			0xFF7F
#define	HCI_CMD_TSCRIPT				0xFFA0
#define	HCI_CMD_BULK_CMD2MODEL		0xFFA1
#define	HCI_CMD_BULK_SET_PAR		0xFFA2			// to VC node
#define	HCI_CMD_BULK_CMD2DEBUG		0xFFA3
#define	HCI_CMD_ADV_PKT				0xFFA4			// to bear and GATT
#define	HCI_CMD_ADV_DEBUG_MESH_DONGLE2BEAR	0xFFA5	// only to bear
#define	HCI_CMD_ADV_DEBUG_MESH_DONGLE2GATT	0xFFA6	// only to gatt
#define	HCI_CMD_ADV_DEBUG_MESH_LED	0xFFA7		// control LED
#define	HCI_CMD_BULK_SET_PAR2USB	0xFFA8
#define	HCI_CMD_SET_VC_PAR			0xFFAA
		
//
#define	HCI_CMD_KMA_DONGLE_SET_MAC  			0xFFE0
#define	HCI_CMD_KMA_DONGLE_SPP_TEST_DATA		0xFFE1
#define	HCI_CMD_KMA_DONGLE_SPP_TEST_STATUS  	0xFFE2
#define	HCI_CMD_KMA_DONGLE_SET_ADV_REPORT_EN	0xFFE3
#define	HCI_CMD_USER_DEFINE			0xFFE4
#define	HCI_CMD_GATEWAY_CMD			0xFFE8
#define	HCI_CMD_GATEWAY_CTL			0xFFE9
#define	HCI_CMD_GATEWAY_OTA			0xFFEA
#define	HCI_CMD_MESH_OTA			0xFFEB
#define	HCI_CMD_PROVISION			0xFEA0

// opcode of button



extern bool MODE_PROVISION;
extern bool MODE_MQTT;
extern bool MODE_BUTTON;
extern bool MODE_CONTROL;

//message control
extern uint8_t OUTMESSAGE_ScanStop[3];
extern uint8_t OUTMESSAGE_ScanStart[3];
extern uint8_t OUTMESSAGE_MACSelect[9];
extern uint8_t OUTMESSAGE_GetPro[3];
extern uint8_t OUTMESSAGE_Provision[28];
extern uint8_t OUTMESSAGE_BindingALl[22];

#ifdef __cplusplus
}
#endif

#endif
