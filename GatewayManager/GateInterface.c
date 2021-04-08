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
//uint16_t j;

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
	else{
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
				for(i=0; i<6; i++){
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
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_SETPRO_SUSCESS){
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
					for (i=0;i<23;i++){
						OUTMESSAGE_Provision[i+3]=vrts_GWIF_IncomeMessage->Message[i+2];
					}
					OUTMESSAGE_Provision[26] = 0x02;
					OUTMESSAGE_Provision[27] = 0x00;
					adr_heartbeat= 2;
				}
				else{
					for (i=0;i<25;i++){
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
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_SEND_ELE_CNT){
				flag_getpro_element=true;
			}
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_PROVISION_EVT && vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_PROVISION_SUSCESS){
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
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_CMD_KEY_BIND_EVT && vrts_GWIF_IncomeMessage->Message[1] == HCI_GATEWAY_CMD_BIND_SUSCESS){
				slog_info("<provision> success");
				// for gpio
				flag_blink = false;
				pthread_cancel(tmp1);
				led_pin_on(gpio[LED_BLE_PIN_INDEX]);
			}
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_KEY_BIND_RSP){
				flag_set_type = true;
			}
			if(vrts_GWIF_IncomeMessage->Message[0] == TSCRIPT_HEARBEAT){
				uint16_t adr = vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8);
				json_component jsonAdr = {"ADR",adr,json_type_int};
				json_component status = {"STATUS","ONLINE",json_type_string};
				create_json_obj_from(add_component_to_obj, 2,mqtt_push, &jsonAdr, &status);
				flag_checkHB = true;
			}
            /*...........................................*/
/*
 * TODO: process for sensor(light, PIR, remote,...)
 */
			if(vrts_GWIF_IncomeMessage->Message[0]==HCI_GATEWAY_RSP_OP_CODE && vrts_GWIF_IncomeMessage->Message[5] == (SENSOR_TYPE & 0xFF)){
				uint16_t adr = (vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8));
				uint16_t headerSensor = vrts_GWIF_IncomeMessage->Message[6] | (vrts_GWIF_IncomeMessage->Message[7]<<8);

				json_component jsonAdr = {"ADR",adr,json_type_int};
				if((headerSensor == REMOTE_MODULE_DC_TYPE) || (headerSensor == REMOTE_MODULE_AC_TYPE)){
					vrts_Remote_Rsp = (remotersp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t pscenedc = (vrts_Remote_Rsp->senceID[0]) |(vrts_Remote_Rsp->senceID[1]<<8);
					uint16_t srgbid = (vrts_Remote_Rsp->futureID[0]) |(vrts_Remote_Rsp->futureID[1]<<8);
					uint8_t *buttonId_String;
					if(vrts_Remote_Rsp->buttonID == 1){
						buttonId_String = "BUTTON_1";
					}
					else if(vrts_Remote_Rsp->buttonID == 2){
						buttonId_String = "BUTTON_2";
					}
					else if(vrts_Remote_Rsp->buttonID == 3){
						buttonId_String = "BUTTON_3";
					}
					else if(vrts_Remote_Rsp->buttonID == 4){
						buttonId_String = "BUTTON_4";
					}
					else if(vrts_Remote_Rsp->buttonID == 5){
						buttonId_String = "BUTTON_5";
					}
					else if(vrts_Remote_Rsp->buttonID == 6){
						buttonId_String = "BUTTON_6";
					}
					json_component cmd = {"CMD","BUTTON",json_type_string};
					json_component button = {"BUTTONID", buttonId_String, json_type_string};
					json_component mode = {"MODEID", vrts_Remote_Rsp->modeID, json_type_int};
					json_component scene = {"SCENEID", pscenedc, json_type_int};
					json_component srgbid_push = {"SRGBID",srgbid,json_type_int};
					create_json_obj_from(add_component_to_obj, 6,mqtt_push,&cmd, &jsonAdr, &button, &mode, &scene, &srgbid_push);
					if(pscenedc!=0){
						/*call scene RGB*/
						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
								NULL16, NULL16,NULL16, NULL16,pscenedc, NULL8, NULL8, NULL8, NULL8,23);
						sleep(2);
						/*call scene normal*/
						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,pscenedc, NULL16,NULL16, NULL16, NULL16, 17);
					}
				}
				else if (headerSensor == POWER_TYPE){
					vrts_Battery_Rsp = (batteryRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t power = ProcessBat(vrts_Battery_Rsp);

					json_component jsonPower = {"POWER",power,json_type_int};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &jsonAdr, &jsonPower);
				}
				else if (headerSensor == LIGHT_SENSOR_MODULE_TYPE){
					vrts_LighSensor_Rsp = (lightsensorRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					ProcessLightSensor(vrts_LighSensor_Rsp);

					json_component jsonLux = {"LUX",value_Lux,json_type_int};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &jsonAdr, &jsonLux);
					uint16_t rspSceneSensor = vrts_LighSensor_Rsp->future[0] | vrts_LighSensor_Rsp->future[1]<<8;
					if(rspSceneSensor != 0){
						/*call scene RGB*/
						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
								NULL16, NULL16,NULL16, NULL16,rspSceneSensor, NULL8, NULL8, NULL8, NULL8,23);
						sleep(2);
						/*call scene normal*/
						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,rspSceneSensor, NULL16,NULL16, NULL16, NULL16, 17);
					}
				}
				else if (headerSensor == PIR_SENSOR_MODULE_TYPE){
					vrts_PirSensor_Rsp = (pirsensorRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t motion = vrts_PirSensor_Rsp->pir[0] | vrts_PirSensor_Rsp->pir[1]<<8;
					uint8_t jsonMotion;
					if(motion == 1){
						jsonMotion = 1;
					}
					else if(motion == 2){
						jsonMotion = 0;
					}
					json_component jsonPir = {"PIR",jsonMotion,json_type_int};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &jsonAdr, &jsonPir);

					uint16_t rspSceneSensor = vrts_PirSensor_Rsp->future[0] | vrts_PirSensor_Rsp->future[1]<<8;
					if(rspSceneSensor != 0){
						/*call scene RGB*/
						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
								NULL16, NULL16,NULL16, NULL16,rspSceneSensor, NULL8, NULL8, NULL8, NULL8,23);
						sleep(2);
						/*call scene normal*/
						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,rspSceneSensor, NULL16,NULL16, NULL16, NULL16, 17);
					}
				}
				else if(headerSensor == PM_SENSOR_HEADER){
					vrts_PMSensor_Rsp = (pmsensorRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					switch (vrts_PMSensor_Rsp->typeValue){
					case PM10_SENSOR_TYPEVALUE:
						break;
					case PM2_5_SENSOR_TYPEVALUE:
						break;
					case PM1_0_SENSOR_TYPEVALUE:
						break;
					}
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
				json_component adr = {"ADR",jsonadr,json_type_int};
				switch (valueOpcode){
				case G_ONOFF_STATUS:
					if(vrui_GWIF_LengthMeassge == 9){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] & 0xFF;
					}
					else{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
					}
					json_component onoff = {"ONOFF",jsonvalue,json_type_int};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &onoff);
					break;
				case LIGHT_CTL_TEMP_STATUS:
					if(vrui_GWIF_LengthMeassge == 12){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] | (vrts_GWIF_IncomeMessage->Message[8] <<8);
					}
					else{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
					}
					json_component cct = {"CCT",Param2PrecentCCT(jsonvalue),json_type_int};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &cct);
					break;
				case LIGHTNESS_STATUS:
					if(vrui_GWIF_LengthMeassge == 10){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
					}
					if(vrui_GWIF_LengthMeassge == 13){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
					}
					json_component dim = {"DIM",Param2PrecentDIM(jsonvalue),json_type_int};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &dim);
					break;

				case LIGHT_HSL_STATUS:
					h = vrts_GWIF_IncomeMessage->Message[9]  | (vrts_GWIF_IncomeMessage->Message[10]<<8);
					s = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
					l = vrts_GWIF_IncomeMessage->Message[7]  | (vrts_GWIF_IncomeMessage->Message[8]<<8);
					json_component hue = {"HUE",h,json_type_int};
					json_component saturation = {"SATURATION",s,json_type_int};
					json_component lightness = {"LIGHTNESS",l,json_type_int};
					create_json_obj_from(add_component_to_obj, 4,mqtt_push, &adr, &hue, &saturation, &lightness);
					break;

				case CFG_MODEL_SUB_STATUS:
					jsonvalue = vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8);
					if(check_add_or_del_group){
						json_component addgroup = {"ADDGROUP", jsonvalue,json_type_int};
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &addgroup);
					}
					else {
						json_component delgroup = {"DELGROUP", jsonvalue,json_type_int};
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &delgroup);
					}
					break;
				case SCENE_REG_STATUS:
					if(check_add_or_del_scene){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8]| vrts_GWIF_IncomeMessage->Message[9]<<8;
						json_component addscene = {"ADDSCENE",jsonvalue,json_type_int};
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &addscene);
					}
					else{
						jsonvalue = vrts_Json_String.delscene;
						json_component delscene = {"DELSCENE",jsonvalue,json_type_int};
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &delscene);
					}
					break;
				case SCENE_STATUS:
					if(vrui_GWIF_LengthMeassge == 13){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[9] | vrts_GWIF_IncomeMessage->Message[10]<<8;
					}
					else{
						jsonvalue = vrts_GWIF_IncomeMessage->Message[7] | vrts_GWIF_IncomeMessage->Message[8]<<8;
					}
					json_component callscene = {"CALLSCENE",jsonvalue,json_type_int};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &callscene);
					break;
				case NODE_RESET_STATUS:
					if(1){
						json_component cmd = {"CMD","RESETNODE",json_type_string};
						json_component jsonAdr = {"ADR",jsonadr,json_type_int};
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &jsonAdr);
					}
					break;
				}
			}
/*
 * TODO:opcode vendor
 */
			if(vrts_GWIF_IncomeMessage->Message[0] == HCI_GATEWAY_RSP_OP_CODE)
			{
				uint16_t jsonAdr =  (vrts_GWIF_IncomeMessage->Message[1]) | (vrts_GWIF_IncomeMessage->Message[2]<<8);
				uint16_t jsonAdrgw= vrts_GWIF_IncomeMessage->Message[3] | (vrts_GWIF_IncomeMessage->Message[4]<<8);
				uint8_t opcodevendor = vrts_GWIF_IncomeMessage->Message[5];
				uint16_t vendorid = (vrts_GWIF_IncomeMessage->Message[6])|(vrts_GWIF_IncomeMessage->Message[7]<<8);
				uint16_t header = (vrts_GWIF_IncomeMessage->Message[8])|(vrts_GWIF_IncomeMessage->Message[9]<<8);
				json_component adr = {"ADR",jsonAdr,json_type_int};

				if((opcodevendor == RD_OPCODE_TYPE_RSP) && (vendorid == VENDOR_ID))
				{
					if((header == HEADER_TYPE_ASK) || (header == HEADER_TYPE_SET))
					{
						if(flag_typeDEV){
							flag_typeDEV = false;
							flag_checkTypeDEV = true;
							//puts("vietanh");
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
							fclose(file);
							/**/
							json_component cmd = {"CMD","TYPE_DEVICE",json_type_string};
							json_component json_unicast_id = {"DEVICE_UNICAST_ID",jsonAdr,json_type_int};
							json_component json_id = {"DEVICE_ID",uuid,json_type_string};
							json_component json_device_key = {"DEVICE_KEY",device_key,json_type_string};
							json_component json_net_key = {"NET_KEY",net_key,json_type_string};
							json_component json_app_key = {"APP_KEY",app_key,json_type_string};
							json_component json_type_id = {"DEVICE_TYPE_ID",TypeConvertID(jsonType,jsonAttrubute,jsonApplication),json_type_int};
							json_object *data = create_json_obj_from(add_component_to_obj, 6,mqtt_dont_push, &json_unicast_id, &json_id, &json_device_key, &json_net_key, &json_app_key, &json_type_id);
							json_component jsondata = {"DATA",data,json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &jsondata);
						}
					}
					else if(header == HEADER_TYPE_SAVEGW){
						if(flag_saveGW){
							flag_saveGW = false;
							flag_checkSaveGW = true;
							json_component savegateway = {"SAVEGATEWAY",jsonAdrgw,json_type_int};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &savegateway);
						}
					}
				}
				if((opcodevendor == RD_OPCODE_SCENE_RSP) && (vendorid == VENDOR_ID))
				{
					uint8_t rspButtonId		= vrts_GWIF_IncomeMessage->Message[10];
					uint8_t rspModeId		= vrts_GWIF_IncomeMessage->Message[11];
					uint16_t rspSceneRgb 	= vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8);
					uint16_t rspSceneRemote = vrts_GWIF_IncomeMessage->Message[12] | (vrts_GWIF_IncomeMessage->Message[13]<<8);
					uint16_t rspSceneSensor = vrts_GWIF_IncomeMessage->Message[19] | (vrts_GWIF_IncomeMessage->Message[20]<<8);
					uint8_t rspSTT 			= vrts_GWIF_IncomeMessage->Message[10];
					uint16_t rspCondition 	= (vrts_GWIF_IncomeMessage->Message[11]<<8) | vrts_GWIF_IncomeMessage->Message[12];
					uint16_t rspLowLux		= vrts_GWIF_IncomeMessage->Message[13] | (vrts_GWIF_IncomeMessage->Message[14]<<8);
					uint16_t rspHighLux		= vrts_GWIF_IncomeMessage->Message[15] | (vrts_GWIF_IncomeMessage->Message[16]<<8);
					uint16_t rspActiontime 	= vrts_GWIF_IncomeMessage->Message[17] | (vrts_GWIF_IncomeMessage->Message[18]<<8);
					uint8_t rspSrgbIdRemote	= vrts_GWIF_IncomeMessage->Message[16];
					uint8_t rspSrgbIdSensor	= vrts_GWIF_IncomeMessage->Message[23];
					uint8_t modeRgb			= vrts_GWIF_IncomeMessage->Message[12];

					json_component callmodergb = {"CALLMODERGB",modeRgb,json_type_int};
					json_component delscenergb = {"DELSCENERGB",rspSceneRgb,json_type_int};
					json_component setscenergb = {"SETSCENERGB",rspSceneRgb,json_type_int};
					json_component callscenergb = {"CALLSCENERGB",rspSceneRgb,json_type_int};

					uint8_t *rspBID_String;
					if(rspButtonId == 1){
						rspBID_String = "BUTTON_1";
					}
					else if(rspButtonId ==2){
						rspBID_String = "BUTTON_2";
					}
					else if(rspButtonId ==3){
						rspBID_String = "BUTTON_3";
					}
					else if(rspButtonId ==4){
						rspBID_String = "BUTTON_4";
					}
					else if(rspButtonId == 5){
						rspBID_String = "BUTTON_5";
					}
					else if(rspButtonId ==6){
						rspBID_String = "BUTTON_6";
					}
					json_component btid = {"BUTTONID",rspBID_String,json_type_string};
					json_component modid = {"MODEID",rspModeId,json_type_int};
					json_component sceneidR = {"SCENEID",rspSceneRemote,json_type_int};
					json_component srgbidR = {"SRGBID",rspSrgbIdRemote,json_type_int};

					json_component jsonStt = {"STT",rspSTT,json_type_int};
					json_component jsonSceneDel = {"SCENEID",rspSTT,json_type_int};
					json_component jsonCondition = {"CONDITION",rspCondition,json_type_int};
					json_component jsonLowLux = {"LOW_LUX",rspLowLux,json_type_int};
					json_component jsonHighLux = {"HIGHT_LUX",rspHighLux,json_type_int};
					json_component jsonAction = {"ACTION",rspActiontime,json_type_int};
					json_component jsonSceneS = {"SCENEID",rspSceneSensor,json_type_int};
					json_component jsonSrgbidS = {"SRGBID",rspSrgbIdSensor,json_type_int};
					switch(header){
					case HEADER_SCENE_CALL_MODE:
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &callmodergb);
						break;
					case HEADER_SCENE_DEL:
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &delscenergb);
						break;
					case HEADER_SCENE_SET:
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &setscenergb);
						break;
					case HEADER_SCENE_CALL_SCENE_RGB:
						create_json_obj_from(add_component_to_obj, 2,mqtt_push, &adr, &callscenergb);
						break;
					case HEADER_SCENE_REMOTE_SET:
						if(1){
							json_component cmd = {"CMD","SETSCENEFORREMOTE",json_type_string};
							create_json_obj_from(add_component_to_obj, 6,mqtt_push,&cmd, &adr, &btid, &modid, &sceneidR, &srgbidR);
						}
						break;
					case HEADER_SCENE_REMOTE_DEL:
						if(1){
							json_component cmd = {"CMD","DELSCENEFORREMOTE",json_type_string};
							create_json_obj_from(add_component_to_obj, 4, mqtt_push, &cmd, &adr, &btid, &modid);
						}
						break;
					case HEADER_SCENE_SENSOR_SET:
						if(1){

							if(rspCondition == 1280 || rspCondition == 1281 || rspCondition == 1282){
								json_component cmd = {"CMD","DELSCENEFORSENSOR",json_type_string};
								create_json_obj_from(add_component_to_obj, 3,mqtt_push, &cmd, &adr, &jsonSceneDel);
							}
							else {
								json_component cmd = {"CMD","SETSCENEFORSENSOR",json_type_string};
								json_component  jsonPir= {"PIR",vrts_Json_String.motion,json_type_int};
								json_object *pir_Sensor_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push,&jsonPir);
								json_component pir_Sensor_push_mqtt = {"PIR_SENSOR",pir_Sensor_object,json_type_object};
								if (rspCondition == 256){
									json_component type = {"TYPE",0,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",4,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_object *light_sensor_object = create_json_obj_from(add_component_to_obj, 2,mqtt_dont_push, &condition_push_mqtt,&low_lux_push_mqtt);
									json_component ligh_sensor_push_mqtt = {"LIGHT_SENSOR",light_sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 6, mqtt_push, &cmd, &adr, &type, &ligh_sensor_push_mqtt, &jsonSceneS, &jsonSrgbidS);
								}
								else if(rspCondition == 512){
									json_component type = {"TYPE",0,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",1,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_object *light_sensor_object = create_json_obj_from(add_component_to_obj, 2,mqtt_dont_push, &condition_push_mqtt,&low_lux_push_mqtt);
									json_component ligh_sensor_push_mqtt = {"LIGHT_SENSOR",light_sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 6, mqtt_push, &cmd, &adr, &type, &ligh_sensor_push_mqtt, &jsonSceneS, &jsonSrgbidS);
								}
								else if(rspCondition == 768){
									json_component type = {"TYPE",0,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",6,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_object *light_sensor_object = create_json_obj_from(add_component_to_obj, 2,mqtt_dont_push, &condition_push_mqtt,&low_lux_push_mqtt);
									json_component ligh_sensor_push_mqtt = {"LIGHT_SENSOR",light_sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 6, mqtt_push, &cmd, &adr, &type, &ligh_sensor_push_mqtt, &jsonSceneS, &jsonSrgbidS);
								}
								else if(rspCondition == 1024){
									json_component type = {"TYPE",0,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",7,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_component hight_lux_push_mqtt = {"HIGHT_LUX",rspHighLux,json_type_int};
									json_object *light_sensor_object = create_json_obj_from(add_component_to_obj, 3,mqtt_dont_push, &condition_push_mqtt,&low_lux_push_mqtt,&hight_lux_push_mqtt);
									json_component ligh_sensor_push_mqtt = {"LIGHT_SENSOR",light_sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 6, mqtt_push, &cmd, &adr, &type, &ligh_sensor_push_mqtt, &jsonSceneS, &jsonSrgbidS);
								}
								else if(rspCondition == 1 || rspCondition == 2){
									json_component type = {"TYPE",1,json_type_int};
									create_json_obj_from(add_component_to_obj, 6, mqtt_push, &cmd,&adr,&type,&pir_Sensor_push_mqtt,&jsonSceneS,&jsonSrgbidS);
								}
								else if(rspCondition == 257 || rspCondition ==258){
									json_component type = {"TYPE",2,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",4,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_object *light_Sensor_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push,&condition_push_mqtt,&low_lux_push_mqtt);
									json_component light_Sensor_push_mqtt = {"LIGHT_SENSOR",light_Sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 7, mqtt_push,&cmd,&adr,&type,&light_Sensor_push_mqtt,&pir_Sensor_push_mqtt,&jsonSceneS,&jsonSrgbidS);
								}
								else if(rspCondition == 513 || rspCondition == 514){
									json_component type = {"TYPE",2,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",1,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_object *light_Sensor_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push,&condition_push_mqtt,&low_lux_push_mqtt);
									json_component light_Sensor_push_mqtt = {"LIGHT_SENSOR",light_Sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 7, mqtt_push,&cmd,&adr,&type,&light_Sensor_push_mqtt,&pir_Sensor_push_mqtt,&jsonSceneS,&jsonSrgbidS);
								}
								else if(rspCondition == 769 || rspCondition == 770){
									json_component type = {"TYPE",2,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",6,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_object *light_Sensor_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push,&condition_push_mqtt,&low_lux_push_mqtt);
									json_component light_Sensor_push_mqtt = {"LIGHT_SENSOR",light_Sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 7, mqtt_push,&cmd,&adr,&type,&light_Sensor_push_mqtt,&pir_Sensor_push_mqtt,&jsonSceneS,&jsonSrgbidS);
								}
								else if(rspCondition == 1025 || rspCondition == 1026){
									json_component type = {"TYPE",2,json_type_int};
									json_component condition_push_mqtt = {"CONDITION",7,json_type_int};
									json_component low_lux_push_mqtt = {"LOW_LUX",rspLowLux,json_type_int};
									json_component hight_lux_push_mqtt = {"HIGHT_LUX",rspHighLux,json_type_int};
									json_object *light_Sensor_object = create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push,&condition_push_mqtt,&low_lux_push_mqtt,&hight_lux_push_mqtt);
									json_component light_Sensor_push_mqtt = {"LIGHT_SENSOR",light_Sensor_object,json_type_object};
									create_json_obj_from(add_component_to_obj, 7, mqtt_push,&cmd,&adr,&type,&light_Sensor_push_mqtt,&pir_Sensor_push_mqtt,&jsonSceneS,&jsonSrgbidS);
								}
							}
						}
						break;
					}
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
		usleep(300);
	}
    return NULL;
}
