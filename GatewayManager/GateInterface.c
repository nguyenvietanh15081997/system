/*
 * GatewayInterface.c
 */

#include "../GatewayManager/GateInterface.h"
#include "../GatewayManager/ButtonManager.h"
#include "../GatewayManager/SensorLight.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/Battery.h"
#include "../GatewayManager/MQTT.h"
#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/slog.h"
#include "../GatewayManager/LedProcess.h"

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

uint8_t uuid[50]="";
uint8_t device_key[50]="";
uint8_t app_key[50]="";
uint8_t net_key[50]="";
uint8_t temp1[2];
uint16_t j;

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
	}
}

/*
 * Ham xu ly du lieu den sau khi da duoc kiem tra
 * Hien tai dung lai o xu ly ban len Terminal
 * TODO: Thuc hien boc ban tin theo cac nhom lenh tuong ung, hoan thien chuan xu ly ban tin den
 */
void GWIF_ProcessData (void)
{
	unsigned int vrui_Count;
	unsigned int i;

	if(vrb_GWIF_CheckNow)
	{
		vrb_GWIF_CheckNow = false;

/*
 * TODO: show data rsp
 */
		uint8_t temDataLog[200];
		uint8_t temp[4];
		uint8_t *temDataUart;
		temDataUart= (uint8_t *)&vrts_GWIF_IncomeMessage->Opcode;
		sprintf(temp,"%x ",vrts_GWIF_IncomeMessage->Opcode);
		strcpy(temDataLog,temp);
		for (vrui_Count = 0; vrui_Count < vrui_GWIF_LengthMeassge-1; vrui_Count++){
			sprintf(temp,"%x ",vrts_GWIF_IncomeMessage->Message[vrui_Count]);
			strcat(temDataLog,temp);
		}
		slog_print(SLOG_INFO, 1, "(rsp)%s",temDataLog);



/*
 * TODO: process for provision
 */

			if((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_UPDATE_MAC) && (flag_check_select_mac == true))
			{  //scan
				for(i=0; i<6; i++)
				{
					OUTMESSAGE_MACSelect[i+3]=vrts_GWIF_IncomeMessage->Message[i+1];
				}
				sprintf(uuid,"%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",vrts_GWIF_IncomeMessage->Message[10],vrts_GWIF_IncomeMessage->Message[11],\
						vrts_GWIF_IncomeMessage->Message[12],vrts_GWIF_IncomeMessage->Message[13],vrts_GWIF_IncomeMessage->Message[14],\
						vrts_GWIF_IncomeMessage->Message[15],vrts_GWIF_IncomeMessage->Message[16],vrts_GWIF_IncomeMessage->Message[17],\
						vrts_GWIF_IncomeMessage->Message[18],vrts_GWIF_IncomeMessage->Message[19],vrts_GWIF_IncomeMessage->Message[20],\
						vrts_GWIF_IncomeMessage->Message[21],vrts_GWIF_IncomeMessage->Message[22],vrts_GWIF_IncomeMessage->Message[23],\
						vrts_GWIF_IncomeMessage->Message[24],vrts_GWIF_IncomeMessage->Message[25]);
				flag_selectmac=true;
				flag_check_select_mac= false;
			}
			if((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PRO_STS_RSP) &&\
					(vrts_GWIF_IncomeMessage->Message[21] != 0x11) &&\
					(vrts_GWIF_IncomeMessage->Message[22] != 0x22) &&\
					(vrts_GWIF_IncomeMessage->Message[23] != 0x33) &&\
					(vrts_GWIF_IncomeMessage->Message[24] != 0x44))
			{
					flag_setpro = true;
			}
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_SETPRO_SUSCESS)
			{
				flag_admitpro = true;
			}
			if((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PRO_STS_RSP) &&\
					(vrts_GWIF_IncomeMessage->Message[21] == 0x11) &&\
					(vrts_GWIF_IncomeMessage->Message[22] == 0x22) &&\
					(vrts_GWIF_IncomeMessage->Message[23] == 0x33) &&\
					(vrts_GWIF_IncomeMessage->Message[24] == 0x44))
			{
				OUTMESSAGE_Provision[0]=HCI_CMD_GATEWAY_CTL;    //0xE9
				OUTMESSAGE_Provision[1]=HCI_CMD_GATEWAY_CTL>>8; //0xFF;
			    OUTMESSAGE_Provision[2]=HCI_GATEWAY_CMD_SET_NODE_PARA;
				if((vrts_GWIF_IncomeMessage->Message[25] == 0x00) &&\
				   (vrts_GWIF_IncomeMessage->Message[26] == 0x00))
				{
					for (i=0;i<23;i++)
					{
						OUTMESSAGE_Provision[i+3]=vrts_GWIF_IncomeMessage->Message[i+2];
					}
					OUTMESSAGE_Provision[26] = 0x02;
					OUTMESSAGE_Provision[27] = 0x00;
					adr_heartbeat= 2;
				}
				else
				{
					for (i=0;i<25;i++)
					{
						OUTMESSAGE_Provision[i+3]=vrts_GWIF_IncomeMessage->Message[i+2];
					}
					adr_heartbeat= OUTMESSAGE_Provision[26] | (OUTMESSAGE_Provision[27]<<8);
				}
				sprintf(net_key,"%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",vrts_GWIF_IncomeMessage->Message[2],vrts_GWIF_IncomeMessage->Message[3],\
										vrts_GWIF_IncomeMessage->Message[4],vrts_GWIF_IncomeMessage->Message[5],vrts_GWIF_IncomeMessage->Message[6],\
										vrts_GWIF_IncomeMessage->Message[7],vrts_GWIF_IncomeMessage->Message[8],vrts_GWIF_IncomeMessage->Message[9],\
										vrts_GWIF_IncomeMessage->Message[10],vrts_GWIF_IncomeMessage->Message[11],vrts_GWIF_IncomeMessage->Message[12],\
										vrts_GWIF_IncomeMessage->Message[13],vrts_GWIF_IncomeMessage->Message[14],vrts_GWIF_IncomeMessage->Message[15],\
										vrts_GWIF_IncomeMessage->Message[16],vrts_GWIF_IncomeMessage->Message[17]);
				flag_getpro_info = true;
			}
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_SEND_ELE_CNT)
			{
				flag_getpro_element=true;
			}

			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PROVISION_EVT && vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_PROVISION_SUSCESS)
			{
				flag_provision =true;
			}

			/* app key*/
			if((vrui_GWIF_LengthMeassge == 27) && (vrts_GWIF_IncomeMessage->Message[0] == 0xb5)){
				sprintf(app_key,"%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",vrts_GWIF_IncomeMessage->Message[10],vrts_GWIF_IncomeMessage->Message[11],\
						vrts_GWIF_IncomeMessage->Message[12],vrts_GWIF_IncomeMessage->Message[13],vrts_GWIF_IncomeMessage->Message[14],\
						vrts_GWIF_IncomeMessage->Message[15],vrts_GWIF_IncomeMessage->Message[16],vrts_GWIF_IncomeMessage->Message[17],\
						vrts_GWIF_IncomeMessage->Message[18],vrts_GWIF_IncomeMessage->Message[19],vrts_GWIF_IncomeMessage->Message[20],\
						vrts_GWIF_IncomeMessage->Message[21],vrts_GWIF_IncomeMessage->Message[22],vrts_GWIF_IncomeMessage->Message[23],\
						vrts_GWIF_IncomeMessage->Message[24],vrts_GWIF_IncomeMessage->Message[25]);
			}

			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_KEY_BIND_EVT && vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_BIND_SUSCESS)
			{
				slog_info("<provision> success");

				// for gpio

				flag_blink = false;
				pthread_cancel(tmp1);
				led_pin_on(gpio[LED_BLE_PIN_INDEX]);

			}
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_KEY_BIND_RSP)
			{
				flag_set_type = true;
			}
            /*...........................................*/
/*
 * TODO: process for sensor(light, PIR, remote,...)
 */
			if(vrts_GWIF_IncomeMessage->Message[0]==HCI_GATEWAY_RSP_OP_CODE && vrts_GWIF_IncomeMessage->Message[5] == (SENSOR_TYPE & 0xFF)){
				uint16_t adr = (vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8));
				if((vrts_GWIF_IncomeMessage->Message[6] == (REMOTE_MODULE_DC_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == (REMOTE_MODULE_DC_TYPE>>8 & 0xFF))){
					puts(">>Remote DC");
					vrts_Remote_Rsp = (remotersp *)(&vrts_GWIF_IncomeMessage->Message[6]);

					uint16_t pscenedc = (vrts_Remote_Rsp->senceID[0] <<8) |(vrts_Remote_Rsp->senceID[1]);
					uint16_t scenrgb = (vrts_Remote_Rsp->futureID[0] <<8) |(vrts_Remote_Rsp->futureID[1]);
					CreatJson_TypeDev(TP_STATUS, "ADR", "BUTTONID", "MODEID", "SCENEID", adr, vrts_Remote_Rsp->buttonID, vrts_Remote_Rsp->modeID,pscenedc);
					if(pscenedc!=0)
					{
						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,pscenedc, NULL16,NULL16, NULL16, NULL16, 17);
						sleep(1);
						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
								NULL16, NULL16,NULL16, NULL16,scenrgb, NULL8, NULL8, NULL8, NULL8,23);
						/*
						 * add to call scene RGB
						 */
					}
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] == (REMOTE_MODULE_AC_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == ((REMOTE_MODULE_AC_TYPE>>8) & 0xFF))){
					puts(">>Remote AC");
					//uint16_t adr = (vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage[2]<<8));
					vrts_Remote_Rsp = (remotersp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t psceneac = (vrts_Remote_Rsp->senceID[0] <<8) |(vrts_Remote_Rsp->senceID[1]);
					uint16_t scenrgb = (vrts_Remote_Rsp->futureID[0] <<8) |(vrts_Remote_Rsp->futureID[1]);
					CreatJson_TypeDev(TP_STATUS, "ADR", "BUTTONID", "MODEID", "SCENEID", adr, vrts_Remote_Rsp->buttonID, vrts_Remote_Rsp->modeID,psceneac);
					if(psceneac!=0)
					{
						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16,\
								NULL16,psceneac, NULL16,NULL16, NULL16, NULL16, 17);
						sleep(1);
						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
														NULL16, NULL16,NULL16, NULL16,scenrgb, NULL8, NULL8, NULL8, NULL8,23);
					}
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] == (POWER_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == ((POWER_TYPE>>8) & 0xFF))){
					puts(">>Power");
					vrts_Battery_Rsp = (batteryRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t power = ProcessBat(vrts_Battery_Rsp);
					CreatJson(TP_STATUS, "ADR", "POWER", adr, power);
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] == (LIGHT_SENSOR_MODULE_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == ((LIGHT_SENSOR_MODULE_TYPE>>8) & 0xFF))){
					puts(">>Light Sensor");
					vrts_LighSensor_Rsp = (lightsensorRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					//uint16_t adrSensorLight= vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
					ProcessLightSensor(vrts_LighSensor_Rsp);
					CreatJson(TP_STATUS, "ADR", "LUX", adr, value_Lux);
				}
				else if ((vrts_GWIF_IncomeMessage->Message[6] == (PIR_SENSOR_MODULE_TYPE & 0xFF)) && \
				   (vrts_GWIF_IncomeMessage->Message[7] == ((PIR_SENSOR_MODULE_TYPE>>8) & 0xFF))){
					puts(">>PIR MOTION");
					//uint16_t adrSensorPIR= vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
					CreatJson(TP_STATUS, "ADR", "PIR", adr, 1);
				}
			}
            /*..........................*/
/*
 * TODO: process of light
 * - check opcode
 * - get status
 * - send mqtt
 */
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_RSP_OP_CODE)
			{
				uint16_t valueOpcode,jsonadr,jsonvalue;
				uint16_t h,s,l;
				valueOpcode = (vrts_GWIF_IncomeMessage->Message[5] | (vrts_GWIF_IncomeMessage->Message[6]<<8));
				jsonadr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
				switch (valueOpcode)
				{
				case G_ONOFF_STATUS:
					if(vrui_GWIF_LengthMeassge == 9){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] & 0xFF;
					}
					else{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
					}
					CreatJson(TP_STATUS,"ADR","ONOFF",jsonadr,jsonvalue);

					break;
				case LIGHT_CTL_TEMP_STATUS:
					if(vrui_GWIF_LengthMeassge == 12){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] | (vrts_GWIF_IncomeMessage->Message[8] <<8);
					}
					else{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
					}
					CreatJson(TP_STATUS,"ADR","CCT",jsonadr,Param2PrecentCCT(jsonvalue));
					break;
				case LIGHTNESS_STATUS:
					if(vrui_GWIF_LengthMeassge == 10){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
					}
					if(vrui_GWIF_LengthMeassge == 13){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
					}
					CreatJson(TP_STATUS, "ADR", "DIM", jsonadr, Param2PrecentDIM(jsonvalue));
					break;

				case LIGHT_HSL_STATUS:
					h = vrts_GWIF_IncomeMessage->Message[9]  | (vrts_GWIF_IncomeMessage->Message[10]<<8);
					s = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
					l = vrts_GWIF_IncomeMessage->Message[7]  | (vrts_GWIF_IncomeMessage->Message[8]<<8);
					CreatJson_TypeDev(TP_STATUS, "ADR","HUE","SATURATION","LIGHTNESS",jsonadr, h, s, l);
					break;

				case CFG_MODEL_SUB_STATUS:
					jsonvalue = vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8);
					if(check_add_or_del_group){
						CreatJson(TP_STATUS, "ADR", "ADDGROUP", jsonadr, jsonvalue);
					}
					else {
						CreatJson(TP_STATUS, "ADR", "DELGROUP", jsonadr, jsonvalue);
					}
					break;
				case SCENE_REG_STATUS:
					if(check_add_or_del_scene){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8]| vrts_GWIF_IncomeMessage->Message[9]<<8;
						CreatJson(TP_STATUS, "ADR", "ADDSCENE", jsonadr, jsonvalue);
					}
					else{
						jsonvalue = vrts_Json_String.delscene;
						CreatJson(TP_STATUS, "ADR", "DELSCENE", jsonadr, jsonvalue);
					}
					break;
				case SCENE_STATUS:
					if(vrui_GWIF_LengthMeassge == 13)
					{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[9] | vrts_GWIF_IncomeMessage->Message[10]<<8;
					}
					else
					{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] | vrts_GWIF_IncomeMessage->Message[8]<<8;
					}
					CreatJson(TP_STATUS, "ADR", "CALLSENCE", jsonadr, jsonvalue);
					break;
				case NODE_RESET_STATUS:
					jsonvalue = 1;
					CreatJsonString(TP_STATUS, "CMD", "ADR", "RESETNODE", jsonadr);
					break;
				}
			}
			if((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_RSP_OP_CODE) && \
					(vrts_GWIF_IncomeMessage->Message[5] == RD_OPCODE_TYPE_RSP) && \
					(((vrts_GWIF_IncomeMessage->Message[6])|(vrts_GWIF_IncomeMessage->Message[7]<<8)) == VENDOR_ID))
			{
				uint16_t jsonadr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
				if(((vrts_GWIF_IncomeMessage->Message[8]|(vrts_GWIF_IncomeMessage->Message[9]<<8)) == HEADER_TYPE_ASK) || \
						((vrts_GWIF_IncomeMessage->Message[8]|(vrts_GWIF_IncomeMessage->Message[9]<<8)) == HEADER_TYPE_SET)){
					uint8_t jsonType,jsonAttrubute,jsonApplication;
					jsonType = vrts_GWIF_IncomeMessage->Message[10];
					jsonAttrubute = vrts_GWIF_IncomeMessage->Message[11];
					jsonApplication = vrts_GWIF_IncomeMessage->Message[12];

					/*read device key*/
					FILE * file;
					if ((file = fopen("/root/device_key.txt","r")) == NULL){
					       printf("Error! opening file");
					       exit(1);
					}
					fscanf(file,"%[^\n]",device_key);
					//puts(device_key);
					fclose(file);
					/**/

					CreatJson_New_TypeDev(TP_STATUS, "DEVICE_UNICAST_ID", "DEVICE_ID", "DEVICE_KEY", "NET_KEY", "APP_KEY", "DEVICE_TYPE_ID", "CMD", "DATA", jsonadr, uuid, device_key, net_key, app_key, TypeConvertID(jsonType,jsonAttrubute,jsonApplication), "TYPE_DEVICE");
				}
				else if((vrts_GWIF_IncomeMessage->Message[8]|(vrts_GWIF_IncomeMessage->Message[9]<<8)) == HEADER_TYPE_SAVEGW){
					CreatJson(TP_STATUS, "ADR", "SAVEGATEWAY", jsonadr, 1);
				}
			}
			if((vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_RSP_OP_CODE) && \
					(vrts_GWIF_IncomeMessage->Message[5] == RD_OPCODE_SCENE_RSP) && \
					(((vrts_GWIF_IncomeMessage->Message[6]>>8)|vrts_GWIF_IncomeMessage->Message[7]) == VENDOR_ID))
			{
				uint16_t jsonadr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
				uint16_t header_scene= vrts_GWIF_IncomeMessage->Message[8]| (vrts_GWIF_IncomeMessage->Message[9]<<8);
				switch(header_scene)
				{
				case HEADER_SCENE_CALL_MODE:
					CreatJson(TP_STATUS, "ADR", "CALLMODERGB",jsonadr , vrts_GWIF_IncomeMessage->Message[12]);
					break;
				case HEADER_SCENE_DEL:
					CreatJson(TP_STATUS, "ADR", "DELSCENERGB", jsonadr, (vrts_GWIF_IncomeMessage->Message[10]>>8)|vrts_GWIF_IncomeMessage->Message[11]);
					break;
				case HEADER_SCENE_SET:
					CreatJson(TP_STATUS, "ADR","SETSCENERGB" , jsonadr,(vrts_GWIF_IncomeMessage->Message[10]>>8)|vrts_GWIF_IncomeMessage->Message[11]);
					break;
				case HEADER_SCENE_CALL_SCENE_RGB:
					CreatJson(TP_STATUS,"ADR","CALLSCENERGB", jsonadr,(vrts_GWIF_IncomeMessage->Message[10]>>8)|vrts_GWIF_IncomeMessage->Message[11] );
					break;
				case HEADER_SCENE_REMOTE_SET:
					CreatJson(TP_STATUS, "ADR","SCENEFORREMOTE", jsonadr, 1);
					break;
				case HEADER_SCENE_SENSOR_SET:
					CreatJson(TP_STATUS, "ADR", "SCENEFORSENSOR", jsonadr, 1);
					break;
				}
			}
			if(vrts_GWIF_IncomeMessage->Message[0] == TSCRIPT_HEARBEAT){
				uint16_t adr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
				CreatJsonString_2(TP_STATUS, "ADR", "STATUS", adr,"ONLINE");
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
