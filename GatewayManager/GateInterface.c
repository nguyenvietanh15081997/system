/*
 * GateInterface.c
 *
 *  Created on: Oct 21, 2020
 *      Author: haha
 */

#include "../GatewayManager/GateInterface.h"
#include "../GatewayManager/ButtonManager.h"
#include "../GatewayManager/SensorLight.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/Battery.h"
#include "../GatewayManager/MQTT.h"

static ringbuffer_t 		vrts_ringbuffer_Data;
static mraa_uart_context	vrts_UARTContext;
static unsigned char 		vruc_GWIF_InUARTData;
static TS_GWIF_IncomingData	*vrts_GWIF_IncomeMessage;
static unsigned char		vrsc_GWIF_TempBuffer[TEMPBUFF_LENGTH] = {0};
static uint16_t				vrui_GWIF_LengthMeassge;
//static uint16_t				vrui_GWIF_LengthMeassgeSave;
static bool					vrb_GWIF_UpdateLate = false;
static bool					vrb_GWIF_CheckNow = false;
static bool					vrb_GWIF_RestartMessage = true;



//unsigned int Timeout_CheckDataBuffer=0;


/*
 * Khoi tao chuong trinh giao tiep vooi Gateway bao gom:
 * - Khoi tao UART
 * - Khoi tao chuoi luu tru du lieu
 */
void GWIF_Init (void){
	ring_init(&vrts_ringbuffer_Data, RINGBUFFER_LEN, sizeof(uint8_t));
	vrts_UARTContext = mraa_uart_init(UART_INTERFACE);
	mraa_uart_set_baudrate(vrts_UARTContext, UART_BAUDRATE);
	mraa_uart_set_mode(vrts_UARTContext, UART_LENGTHDATA, MRAA_UART_PARITY_NONE, 1);
	mraa_uart_set_flowcontrol(vrts_UARTContext, 0, 0);
	// Dua con tro du lieu ve mang du lieu nhan ve
	vrts_GWIF_IncomeMessage = (TS_GWIF_IncomingData *)vrsc_GWIF_TempBuffer;
}


/*
 * Ham xu ly truyen ban tin tu cac tien trinh khac den Gateway
 * Hien tai dang cho phep xu ly toi da 1 ban tin den dung semaphore
 * TODO: Can nang cap len Semaphore ho tro nhieu ban tin den
 */
void GWIF_WriteMessage (void){
	if(pthread_mutex_trylock(&vrpth_SHAREMESS_Send2GatewayLock) == 0){
		if(vrb_SHAREMESS_Send2GatewayAvailabe == true){
			vrb_SHAREMESS_Send2GatewayAvailabe = false;
			mraa_uart_write(vrts_UARTContext, (const char *)vrsc_SHAREMESS_Send2GatewayMessage, vrui_SHAREMESS_Send2GatewayLength);
			//printf("Put to Gateway\n");
		}
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
	}
}

/*
 * Doc du lieu tu bo dem UART va luu tru vao Ring Buffer cua he thong
 */
void GWIF_Read2Buffer (void){
	// Neu hang doi chua day Hoac Du lieu con trong UART thi dua du lieu do vao hang doi
	while((vrts_ringbuffer_Data.count < RINGBUFFER_LEN) && mraa_uart_data_available(vrts_UARTContext, 0)){
		mraa_uart_read(vrts_UARTContext,(char *)(&vruc_GWIF_InUARTData), 1);
		ring_push_head(&vrts_ringbuffer_Data, (void*)&vruc_GWIF_InUARTData);
	}
}

/*
 * Kiem tra tinh xac thuc cua du lieu den
 * Neu du lieu dung format thi lay ra so byte tuong ung va gui sang tien trinh xu ly
 * Neu chua dung thi tiep tuc dich du lieu va tim den doan du lieu dung format
 * REVIEW: Kiem tra lai noi dung ham nay
 */
void GWIF_CheckData (void){
	unsigned int vrui_Count;

	// Neu co du lieu trong Buffer
	if(vrts_ringbuffer_Data.count >= 1){
		scanNotFoundDev = 0;
		Timeout_CheckDataBuffer=0;
		if(vrb_GWIF_UpdateLate == false){

			// Doc du lieu vao Buffer roi chuyen du lieu di tiep
			if(vrb_GWIF_RestartMessage == true){
				if(vrts_ringbuffer_Data.count >= 3){
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[0]);
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[1]);
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[2]);
					vrb_GWIF_RestartMessage = false;
					vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
				}
			}
			else{
				ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH - 1]);
				vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
			}

			if((vrui_GWIF_LengthMeassge) >= MESSAGE_HEADLENGTH){
				if(	(vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_MESH_RX)          || \
					(vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_MESH_RX_NW)       || \
					(vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_GATEWAY_DIR_RSP)  || \
					(vrts_GWIF_IncomeMessage->Opcode == HCI_GATEWAY_CMD_SAR_MSG)  || \
					(vrts_GWIF_IncomeMessage->Opcode == TSCRIPT_CMD_VC_DEBUG) ){
					// Truong hop dung format ban tin
					if(vrts_ringbuffer_Data.count >= (vrui_GWIF_LengthMeassge - 1)){
						for(vrui_Count = 0; vrui_Count < (vrui_GWIF_LengthMeassge - 1); vrui_Count++){
							ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH + vrui_Count]);
						}
						// Ban tin da trung khop cau truc, bat co xu ly ban tin
						vrb_GWIF_UpdateLate = false;
						vrb_GWIF_CheckNow = true;
						vrb_GWIF_RestartMessage = true;
						//GWIF_ProcessData();
					}
					else{
						// Neu du lieu den chua duoc chua trong Buffer, tien hanh quet vao lan sau
						vrb_GWIF_UpdateLate = true;
					}
				}
				else{
					// Truong hop khong dung format ban tin
					// dich di 1 Byte va tiep tuc kiem tra
					vrsc_GWIF_TempBuffer[0] = vrsc_GWIF_TempBuffer[1];
					vrsc_GWIF_TempBuffer[1] = vrsc_GWIF_TempBuffer[2];
					vrb_GWIF_RestartMessage = false;
					vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
				}
			}
		}
		else{
			if(vrts_ringbuffer_Data.count >= (vrui_GWIF_LengthMeassge - 1)){
				for(vrui_Count = 0; vrui_Count < (vrui_GWIF_LengthMeassge - 1); vrui_Count++){
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH + vrui_Count]);
				}
				vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
				// Ban tin da trung khop cau truc, bat co xu ly ban tin
				vrb_GWIF_UpdateLate = false;
				vrb_GWIF_CheckNow = true;
				vrb_GWIF_RestartMessage = true;
				//GWIF_ProcessData();
			}
		}

	}
	else
	{
		Timeout_CheckDataBuffer++;
//		if(Timeout_CheckDataBuffer == 3199){
//			scanNotFoundDev++;
//			printf("%d",scanNotFoundDev);
//		}
	}

}

/*
 * Ham xu ly du lieu den sau khi da duoc kiem tra
 * Hien tai dung lai o xu ly ban len Terminal
 * TODO: Thuc hien boc ban tin theo cac nhom lenh tuong ung, hoan thien chuan xu ly ban tin den
 */
void GWIF_ProcessData (void){
	unsigned int vrui_Count;
	unsigned int i;
	if(vrb_GWIF_CheckNow){
		vrb_GWIF_CheckNow = false;

/*
 * TODO: Display data rsp
 */
		printf("A coming message:\n");
		printf("\tLength:%d\n",vrui_GWIF_LengthMeassge);
		printf("\tTSCRIPT:0x%2x\n",vrts_GWIF_IncomeMessage->Opcode);
		printf("\tMessage:");
		for (vrui_Count = 0; vrui_Count < vrui_GWIF_LengthMeassge-1; vrui_Count++){
			printf("%2x-",vrts_GWIF_IncomeMessage->Message[vrui_Count]);
		}
		printf("\n");
        /*............*/


/*
 * TODO: process for provision
 */

			if((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_UPDATE_MAC) && (flag_check_select_mac == true)){  //scan
				for(i=0; i<6; i++)
				{
					OUTMESSAGE_MACSelect[i+3]=vrts_GWIF_IncomeMessage->Message[i+1];
				}

				flag_selectmac=true;
				flag_check_select_mac= false;
			}

			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PRO_STS_RSP){

				OUTMESSAGE_Provision[0]=HCI_CMD_GATEWAY_CTL;    //0xE9
				OUTMESSAGE_Provision[1]=HCI_CMD_GATEWAY_CTL>>8; //0xFF;
				OUTMESSAGE_Provision[2]=HCI_GATEWAY_CMD_SET_NODE_PARA;
				//unicast_address = (vrts_GWIF_IncomeMessage->Message[25]) | (vrts_GWIF_IncomeMessage->Message[26]<<8);
				for (i=0;i<25;i++)
				{
					OUTMESSAGE_Provision[i+3]=vrts_GWIF_IncomeMessage->Message[i+2];
				}
				flag_getpro_info=true;
			}

			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_SEND_ELE_CNT)
			{
				flag_getpro_element=true;
			}

			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PROVISION_EVT && vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_PROVISION_SUSCESS)
			{
				flag_provision =true;
			}

			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_KEY_BIND_EVT && vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_BIND_SUSCESS)
			{
				printf("suscess!\n");
			}
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_KEY_BIND_RSP)
			{
				flag_done=true;
				flag_mac=true;
			}
            /*...........................................*/
			if(vrts_GWIF_IncomeMessage->Message)
/*
 * TODO: process for sensor(light, PIR, remote,...)
 */
			if(vrts_GWIF_IncomeMessage->Message[5] == (SENSOR_TYPE & 0xFF)){
				if((vrts_GWIF_IncomeMessage->Message[6] == (REMOTE_MODULE_DC_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == (REMOTE_MODULE_DC_TYPE>>8) & 0xFF)){
					puts(">>Remote DC");
					vrts_Remote_Rsp = (remotersp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					if(IsRemoteSetup(vrts_Remote_Rsp, 1, 1, 0, 0)){
						//add code at here
					}
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] == (REMOTE_MODULE_AC_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == ((REMOTE_MODULE_AC_TYPE>>8) & 0xFF))){
					puts(">>Remote AC");
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] == (POWER_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == ((POWER_TYPE>>8) & 0xFF))){
					puts(">>Power");
					vrts_Battery_Rsp = (batteryRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					ProcessBat(vrts_Battery_Rsp);
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] == (LIGHT_SENSOR_MODULE_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == ((LIGHT_SENSOR_MODULE_TYPE>>8) & 0xFF))){
					puts(">>Light Sensor");
					vrts_LighSensor_Rsp = (lightsensorRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					ProcessLightSensor(vrts_LighSensor_Rsp);
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] = (PIR_SENSOR_MODULE_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] = ((PIR_SENSOR_MODULE_TYPE>>8) & 0xFF))){
					puts(">>PIR Sensor");

				}
			}
            /*..........................*/
/*
 * TODO: process of light
 * - check opcode
 * - get status
 * - send mqtt
 */
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_RSP_OP_CODE){
				uint16_t valueOpcode,jsonadr,jsonvalue;
				uint8_t jsonmain,jsonsub,jsonpower;
				valueOpcode = (vrts_GWIF_IncomeMessage->Message[5] | (vrts_GWIF_IncomeMessage->Message[6]<<8));
				jsonadr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
				switch (valueOpcode){
				case OPCODE_TYPEDEV:
					jsonmain = vrts_GWIF_IncomeMessage->Message[9];
					jsonsub = vrts_GWIF_IncomeMessage->Message[10];
					jsonpower = vrts_GWIF_IncomeMessage->Message[11];
					CreatJson_TypeDev(TP_TPYE_DEVICE, "ADR", "MAINDEVICE", "SUBDEVICE", "POWER", jsonadr, jsonmain, jsonsub, jsonpower);
					break;
				case G_ONOFF_STATUS:
					//jsonadr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
					if(vrui_GWIF_LengthMeassge == 9){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] & 0xFF;
					}
					else{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
					}
					CreatJson(TP_STATUS_UPDATE,"ADR","ONOFF",jsonadr,jsonvalue);
					break;
				case LIGHT_CTL_TEMP_STATUS:
					//jsonadr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
					jsonvalue = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
					//printf ("%d", jsonvalue);
					CreatJson(TP_STATUS_CCT,"ADR","CCT",jsonadr,jsonvalue);
					break;
				case LIGHTNESS_STATUS:
					//jsonadr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
					if(vrui_GWIF_LengthMeassge == 10){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
					}
					if(vrui_GWIF_LengthMeassge == 13){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
					}
					CreatJson(TP_STATUS_DIM, "ADR", "DIM", jsonadr, jsonvalue);
					break;
				case LIGHT_HSL_STATUS:
					//status HSL
					break;
				case CFG_MODEL_SUB_STATUS:
					jsonvalue = vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
					if(check_add_or_del_group){
						CreatJson(TP_STATUS_ADDGROUP, "ADR", "ADDGROUP", jsonadr, jsonvalue);
					}
					else {
						CreatJson(TP_STATUS_DELGROUP, "ADR", "DELGROUP", jsonadr, jsonvalue);
					}
					break;
				case SCENE_REG_STATUS:
					jsonvalue = vrts_GWIF_IncomeMessage->Message[8]| vrts_GWIF_IncomeMessage->Message[9]<<8;
					CreatJson(TP_STATUS_ADDSENCE, "ADR", "ADDSENCE", jsonadr, jsonvalue);
					break;
				case SCENE_STATUS:
					if(vrts_GWIF_IncomeMessage->Message[6]==0){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] | vrts_GWIF_IncomeMessage->Message[8]<<8;
						CreatJson(TP_STATUS_CALLSENCE, "ADR", "CALLSENCE", jsonadr, jsonvalue);
					}
					break;
				case NODE_RESET_STATUS:
					jsonvalue = 1;
					CreatJson(TP_STATUS_RESETNODE, "ADR", "RESET", jsonadr, jsonvalue);
					break;
				}
			}

	}
}


/*
 * Tien trinh xu ly ban tin giao tiep Gateway bao gom
 * - Nhan ban tin ve
 * - Xu ly ban tin ve
 * - Truyen du lieu tu tac vu khac den Gateway
 * TODO: Tiep tuc kiem tra va hoan thien
 */
void *GWINF_Thread(void *vargp)
{
	GWIF_Init();
	while(1){
		GWIF_WriteMessage();
		GWIF_Read2Buffer();
		GWIF_CheckData();
		GWIF_ProcessData();
		usleep(200);
	}
    return NULL;
}
