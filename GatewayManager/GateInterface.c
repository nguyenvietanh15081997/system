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
#include "../GatewayManager/Ds1307.h"
#include "../GatewayManager/Linkerlist.h"

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
static bool 				message_Update = false;

uint8_t uuid[50]="";
uint8_t device_key[50]="";
uint8_t app_key[50]="";
uint8_t net_key[50]="";
bool checkcallscene = false;
uint16_t sceneForCCt;

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
						message_Update = true;
					}
				}
				else{
					ring_pop_tail(&vrts_ringbuffer_Data, (void*)&vrsc_GWIF_TempBuffer[MESSAGE_HEADLENGTH - 1]);
					vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
					message_Update = true;
				}
				if(message_Update){
					message_Update = false;
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
								vrb_GWIF_RestartMessage = false;
							}
						}
						else{
							// Truong hop khong dung format ban tin
							// dich di 1 Byte va tiep tuc kiem tra
							vrsc_GWIF_TempBuffer[0] = vrsc_GWIF_TempBuffer[1];
							vrsc_GWIF_TempBuffer[1] = vrsc_GWIF_TempBuffer[2];
							vrb_GWIF_RestartMessage = false;
							vrb_GWIF_UpdateLate = true;
							vrui_GWIF_LengthMeassge = (vrts_GWIF_IncomeMessage->Length[0]) | (vrts_GWIF_IncomeMessage->Length[1]<<8);
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
				json_component jsonAdr = {"DEVICE_UNICAST_ID",adr,json_type_int};
				json_component cmd = {"CMD","STATUS_ONLINE",json_type_string};
				json_object *data_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &jsonAdr);
				json_component data_json = {"DATA",data_object, json_type_object};
				create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &data_json);
				flag_checkHB = true;
			}
            /*...........................................*/
/*
 * TODO: process for sensor(light, PIR, remote,...)
 */
			if(vrts_GWIF_IncomeMessage->Message[0]==HCI_GATEWAY_RSP_OP_CODE && vrts_GWIF_IncomeMessage->Message[5] == (SENSOR_TYPE & 0xFF)){
				uint16_t adr = (vrts_GWIF_IncomeMessage->Message[1] | (vrts_GWIF_IncomeMessage->Message[2]<<8));

				if(adr == unicastId){
					//pthread_mutex_lock(&vrpth_SHAREMESS_FlagCheckRsp);
					flag_check_rsp = true;
					puts("RSP OF remote sensor");
					//pthread_mutex_unlock(&vrpth_SHAREMESS_FlagCheckRsp);
				}
				uint16_t headerSensor = vrts_GWIF_IncomeMessage->Message[6] | (vrts_GWIF_IncomeMessage->Message[7]<<8);

				json_component jsonAdr = {"DEVICE_UNICAST_ID",adr,json_type_int};
				json_component cmd_Sensor_Json = {"CMD","SENSOR_VALUE",json_type_string};
				if((headerSensor == REMOTE_MODULE_DC_TYPE) || (headerSensor == REMOTE_MODULE_AC_TYPE)){
					vrts_Remote_Rsp = (remotersp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t pscenedc = (vrts_Remote_Rsp->senceID[0]) |(vrts_Remote_Rsp->senceID[1]<<8);
					uint16_t srgbid = (vrts_Remote_Rsp->futureID[0])  |(vrts_Remote_Rsp->futureID[1]<<8);
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
					json_component button = {"BUTTON_VALUE", buttonId_String, json_type_string};
					json_component mode = {"MODE_VALUE", vrts_Remote_Rsp->modeID, json_type_int};
					json_object *data_Remote = create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &jsonAdr, &button, &mode);
					json_component data_Remote_Json = {"DATA",data_Remote,json_type_object};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd_Sensor_Json, &data_Remote_Json);
					if(pscenedc!=0){
						/*call scene normal*/
//						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,pscenedc, NULL16,NULL16, NULL16, NULL16, 17);
//						sleep(1);
						/*call scene RGB*/
//						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
//								NULL16, NULL16,NULL16, NULL16,pscenedc, NULL8, NULL8, NULL8, NULL8,NULL16, 23);
//						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,pscenedc, NULL16,NULL16, NULL16, NULL16, 17);
//						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
//														NULL16, NULL16,NULL16, NULL16,pscenedc, NULL8, NULL8, NULL8, NULL8,23);
						sceneForCCt = pscenedc;
						checkcallscene = true;
					}
				}
				else if(headerSensor == SCREEN_TOUCH_MODULE_TYPE){
					vrts_ScreenT_Rsp = (screenTouch *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t psceneScreenT = (vrts_ScreenT_Rsp->sceneID[0]) |(vrts_ScreenT_Rsp->sceneID[1]<<8);
					if(psceneScreenT)
					{

						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16, psceneScreenT,\
								NULL16,NULL16, NULL16, NULL16, NULL16, 17);
						usleep(400000);
					}
					uint8_t *buttonId_String;
					if(vrts_ScreenT_Rsp->buttonID == 1){
						buttonId_String = "BUTTON_1";
					}
					else if(vrts_ScreenT_Rsp->buttonID == 2){
						buttonId_String = "BUTTON_2";
					}
					else if(vrts_ScreenT_Rsp->buttonID == 3){
						buttonId_String = "BUTTON_3";
					}
					else if(vrts_ScreenT_Rsp->buttonID == 4){
						buttonId_String = "BUTTON_4";
					}
					else if(vrts_ScreenT_Rsp->buttonID == 5){
						buttonId_String = "BUTTON_5";
					}
					else if(vrts_ScreenT_Rsp->buttonID == 6){
						buttonId_String = "BUTTON_6";
					}
					if(vrts_ScreenT_Rsp->aksTime == 0x01){
						Function_Vendor(HCI_CMD_GATEWAY_CMD, SendTimeForScreenT_vendor_typedef, 65535, NULL16, NULL8, NULL8, NULL8, NULL16, NULL16, \
								NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, dataTimeInternet[0], dataTimeInternet[1], NULL8, NULL16, 23);
					}
					json_component button = {"BUTTON_VALUE", buttonId_String, json_type_string};
					json_object *data_Remote = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &jsonAdr, &button);
					json_component data_Remote_Json = {"DATA",data_Remote,json_type_object};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd_Sensor_Json, &data_Remote_Json);

				}
				else if (headerSensor == POWER_TYPE){
					vrts_Battery_Rsp = (batteryRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t power = ProcessBat(vrts_Battery_Rsp);

					json_component jsonPower = {"POWER_VALUE",power,json_type_int};
					json_object *data_Power = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push,&jsonAdr, &jsonPower);
					json_component data_Power_Json = {"DATA",data_Power,json_type_object};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd_Sensor_Json, &data_Power_Json);
				}
				else if (headerSensor == LIGHT_SENSOR_MODULE_TYPE){
					vrts_LighSensor_Rsp = (lightsensorRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					ProcessLightSensor(vrts_LighSensor_Rsp);

					json_component jsonLux = {"LUX_VALUE",value_Lux,json_type_int};
					json_object *data_Lux = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &jsonAdr, &jsonLux);
					json_component data_Lux_Json = {"DATA",data_Lux,json_type_object};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd_Sensor_Json, &data_Lux_Json);
					uint16_t rspSceneSensor = vrts_LighSensor_Rsp->future[0] | vrts_LighSensor_Rsp->future[1]<<8;
					if(rspSceneSensor != 0){
						/*call scene normal*/
//						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,rspSceneSensor, NULL16,NULL16, NULL16, NULL16, 17);
//						sleep(1);
//						/*call scene RGB*/
//						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
//								NULL16, NULL16,NULL16, NULL16,rspSceneSensor, NULL8, NULL8, NULL8, NULL8,23);
						}
				}
				else if (headerSensor == PIR_SENSOR_MODULE_TYPE){
					vrts_PirSensor_Rsp = (pirsensorRsp *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t motion = vrts_PirSensor_Rsp->pir[0] | vrts_PirSensor_Rsp->pir[1]<<8;
					uint8_t jsonMotion;
					if(motion == 1){
						jsonMotion = 1;
					}
					else if(motion == 0){
						jsonMotion = 0;
					}
					json_component jsonPir = {"PIR_VALUE",jsonMotion,json_type_int};
					json_object *data_Pir = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &jsonAdr, &jsonPir);
					json_component data_Pir_Json = {"DATA",data_Pir,json_type_object};
					create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd_Sensor_Json, &data_Pir_Json);
					uint16_t rspSceneSensor = vrts_PirSensor_Rsp->future[0] | vrts_PirSensor_Rsp->future[1]<<8;
					if(rspSceneSensor != 0){
						/*call scene normal*/
//						FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16,rspSceneSensor, NULL16,NULL16, NULL16, NULL16, 17);
//						sleep(1);
//						/*call scene RGB*/
//						Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
//								NULL16, NULL16,NULL16, NULL16,rspSceneSensor, NULL8, NULL8, NULL8, NULL8,23);
						}
				}
				else if(headerSensor == PM_SENSOR_MODULE_TEMP_HUM_TYPE){
					vrts_PMSensor_Rsp_Temp_Hum = (pmsensorRsp_Temp_Hum *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t hum = (vrts_PMSensor_Rsp_Temp_Hum->hum[0] << 8)| vrts_PMSensor_Rsp_Temp_Hum->hum[1];
					int temp;
					if(vrts_PMSensor_Rsp_Temp_Hum->temp[0] == 0xFF){
						temp = (-1)*((vrts_PMSensor_Rsp_Temp_Hum->temp[2]<<8) | vrts_PMSensor_Rsp_Temp_Hum->temp[3]);
					}
					else temp = ((vrts_PMSensor_Rsp_Temp_Hum->temp[2]<<8) | vrts_PMSensor_Rsp_Temp_Hum->temp[3]);
					json_component cmd = {"CMD","PM_SENSOR",json_type_string};
					json_component temp_json = {"TEMPERATURE_VALUE",temp,json_type_int};
					json_component hum_json = {"HUMIDITY_VALUE",hum,json_type_int};
					json_object *data_object= create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &jsonAdr, &temp_json, &hum_json);
					json_component data_json = {"DATA",data_object,json_type_object};
					create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_json);
				}
				else if(headerSensor == PM_SENSOR_MODULE_PM_TYPE){
					vrts_PMSensor_Rsp_PM = (pmsensorRsp_PM *)(&vrts_GWIF_IncomeMessage->Message[6]);
					uint16_t pm2_5 = vrts_PMSensor_Rsp_PM->PM_2_5[0]<<8 | vrts_PMSensor_Rsp_PM->PM_2_5[1];
					uint16_t pm10 = vrts_PMSensor_Rsp_PM->PM_10[0] << 8 | vrts_PMSensor_Rsp_PM->PM_10[1];
					uint16_t pm1 = vrts_PMSensor_Rsp_PM->PM_1_0[0]<<8 | vrts_PMSensor_Rsp_PM->PM_1_0[1];
					json_component cmd = {"CMD","PM_SENSOR",json_type_string};
					json_component pm2_5_json = {"PM2.5_VALUE",pm2_5, json_type_int};
					json_component pm1_json = {"PM1_VALUE",pm1,json_type_int};
					json_component pm10_json = {"PM10_VALUE",pm10,json_type_int};
					json_object * data_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push,&jsonAdr, &pm2_5_json, &pm1_json, &pm10_json);
					json_component data_json = {"DATA",data_object,json_type_object};
					create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_json);
				}
				else if (headerSensor == TEMP_HUM_MODULE_TYPE){
					uint16_t temp_Value_original    = ((vrts_GWIF_IncomeMessage->Message[8]<<8) | vrts_GWIF_IncomeMessage->Message[9]);
					uint16_t temp_Value_Uint16_t 	= ((vrts_GWIF_IncomeMessage->Message[8]<<8) | vrts_GWIF_IncomeMessage->Message[9]) & 0x7FFF;
					uint16_t hum_Value_Uint16_t 	= (vrts_GWIF_IncomeMessage->Message[10]<<8) | vrts_GWIF_IncomeMessage->Message[11];
					uint16_t scene_TEMP_HUM_SENSOR 	= (vrts_GWIF_IncomeMessage->Message[12]<<8) | vrts_GWIF_IncomeMessage->Message[13];

					uint8_t check_temp        	= vrts_GWIF_IncomeMessage->Message[8] & 0x80;
					uint8_t temp_Value_Interger = temp_Value_Uint16_t / 10;
					uint8_t hum_Value_Interger 	= hum_Value_Uint16_t / 10;

					json_component hum_Json = {"HUMIDITY_VALUE", hum_Value_Interger, json_type_int};
					json_object *data_Hum = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &jsonAdr, &hum_Json);
					json_component data_Hum_Json = {"DATA",data_Hum,json_type_object};
					create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd_Sensor_Json, &data_Hum_Json);
					if(!check_temp){
						json_component temp_Json = {"TEMPERATURE_VALUE", temp_Value_Interger, json_type_int};
						json_object *data_TEMP = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &jsonAdr, &temp_Json);
						json_component data_TEMP_Json = {"DATA",data_TEMP,json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd_Sensor_Json, &data_TEMP_Json);
					}
					else {
						json_component temp_Json = {"TEMPERATURE_VALUE", (-1)*temp_Value_Interger, json_type_int};
						json_object *data_TEMP = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &jsonAdr,&temp_Json);
						json_component data_TEMP_Json = {"DATA",data_TEMP,json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd_Sensor_Json, &data_TEMP_Json);
					}
					Function_Vendor(HCI_CMD_GATEWAY_CMD, SendTempHumForScreenT_vendor_typedef, 65535, NULL16, NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, temp_Value_original,\
							hum_Value_Uint16_t, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 23);
				}
				else if(headerSensor == DOOR_SENSOR_MODULE_TYPE){
					uint8_t hang 					= vrts_GWIF_IncomeMessage->Message[8];
					uint8_t door 					= vrts_GWIF_IncomeMessage->Message[9];
					uint16_t sceneDoorSensor 		= vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8);
					json_component hang_Json 		= {"HANG_VALUE",hang,json_type_int};
					json_component door_Json 		= {"DOOR_VALUE",door,json_type_int};
					json_object *door_object 		= create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &jsonAdr, &hang_Json, &door_Json);
					json_component data_Door_Json 	= {"DATA",door_object,json_type_object};
					create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd_Sensor_Json, &data_Door_Json);
				}
				else if(headerSensor == SMOKE_SENSOR_MODULE_TYPE){
					uint8_t smoke 					= vrts_GWIF_IncomeMessage->Message[8];
					json_component smoke_Json 		= {"SMOKE_VALUE",smoke,json_type_int};
					json_object * data_Smoke 		= create_json_obj_from(add_component_to_obj, 2, mqtt_push, &jsonAdr, &smoke_Json);
					json_component data_Smoke_Json 	= {"DATA",data_Smoke,json_type_object};
					create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd_Sensor_Json, &data_Smoke_Json);
				}
				else if(headerSensor == SWITCH4_MODULE_TYPE){
					uint8_t relay1 = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
					uint8_t relay2 = vrts_GWIF_IncomeMessage->Message[9] & 0xFF;
					uint8_t relay3 = vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
					uint8_t relay4 = vrts_GWIF_IncomeMessage->Message[11] & 0xFF;
					json_component cmd 			= {"CMD","SWITCH4",json_type_string};
					json_component relay1_json 	= {"RELAY1", relay1, json_type_int};
					json_component relay2_json 	= {"RELAY2", relay2, json_type_int};
					json_component relay3_json 	= {"RELAY3", relay3, json_type_int};
					json_component relay4_json 	= {"RELAY4", relay4, json_type_int};
					json_object *switch4_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &relay1_json, &relay2_json, &relay3_json, &relay4_json);
					json_component switch4_Json = {"SWITCH_STATUS", switch4_object, json_type_object};
					json_object *data_object 	= create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &jsonAdr, &switch4_Json);
					json_component data_json 	= {"DATA", data_object, json_type_object};
					create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_json);
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
				if(jsonadr == unicastId){
					//pthread_mutex_lock(&vrpth_SHAREMESS_FlagCheckRsp);
					flag_check_rsp = true;
					puts("RSP OF LIGHT");
					//pthread_mutex_unlock(&vrpth_SHAREMESS_FlagCheckRsp);

				}
				json_component adr = {"DEVICE_UNICAST_ID",jsonadr,json_type_int};
				switch (valueOpcode){
				case G_ONOFF_STATUS:
					if(1){
						if(vrui_GWIF_LengthMeassge == 9){
							jsonvalue = vrts_GWIF_IncomeMessage->Message[7] & 0xFF;
						}
						else{
							jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
						}
						json_component cmd 	= {"CMD","ONOFF",json_type_string};
						json_component onoff 		= {"VALUE_ONOFF",jsonvalue,json_type_int};
						json_object * data_object 	= create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &onoff);
						json_component data_Json 	= {"DATA",data_object,json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					break;
				case LIGHT_CTL_TEMP_STATUS:
					if(1){
						if(vrui_GWIF_LengthMeassge == 12){
							jsonvalue = vrts_GWIF_IncomeMessage->Message[7] | (vrts_GWIF_IncomeMessage->Message[8] <<8);
						}
						else{
							jsonvalue = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
						}
						json_component cmd = {"CMD","CCT",json_type_string};
						json_component cct = {"VALUE_CCT",Param2PrecentCCT(jsonvalue),json_type_int};
						json_object * data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &cct);
						json_component data_Json = {"DATA", data_object, json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					break;
				case LIGHTNESS_STATUS:
					if(1){
						if(vrui_GWIF_LengthMeassge == 10){
							jsonvalue = vrts_GWIF_IncomeMessage->Message[8] & 0xFF;
						}
						if(vrui_GWIF_LengthMeassge == 13){
							jsonvalue = vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
						}
						json_component cmd  = {"CMD","DIM",json_type_string};
						json_component dim      = {"VALUE_DIM",Param2PrecentDIM(jsonvalue),json_type_int};
						json_object * data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &dim);
						json_component data_Json = {"DATA",data_object,json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					break;
				case LIGHT_HSL_STATUS:
					if(1){
						h = vrts_GWIF_IncomeMessage->Message[9]  | (vrts_GWIF_IncomeMessage->Message[10]<<8);
						s = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
						l = vrts_GWIF_IncomeMessage->Message[7]  | (vrts_GWIF_IncomeMessage->Message[8]<<8);
						json_component cmd    		= {"CMD","HSL",json_type_string};
						json_component hue  		= {"VALUE_H",h,json_type_int};
						json_component saturation 	= {"VALUE_S",s,json_type_int};
						json_component lightness 	= {"VALUE_L",l,json_type_int};
						json_object *data_object 	= create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &adr, &hue, &saturation, &lightness);
						json_component data_Json 	= {"DATA",data_object,json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					break;
				case CFG_MODEL_SUB_STATUS:
					jsonvalue = vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8);
					if(check_add_or_del_group){
						json_component cmd = {"CMD","ADDGROUP",json_type_string};
						json_component addgroup = {"GROUP_UNICAST_ID", jsonvalue,json_type_int};
						json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &addgroup);
						json_component data_Json = {"DATA",data_object, json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					else {
						json_component cmd = {"CMD","DELGROUP",json_type_string};
						json_component delgroup = {"GROUP_UNICAST_ID", jsonvalue,json_type_int};
						json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &delgroup);
						json_component data_Json = {"DATA",data_object,json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					break;
				case SCENE_REG_STATUS:
					if(check_add_or_del_scene){
						jsonvalue = vrts_GWIF_IncomeMessage->Message[8]| vrts_GWIF_IncomeMessage->Message[9]<<8;
						json_component cmd = {"CMD","ADDSCENE", json_type_string};
						json_component addscene = {"SCENEID",jsonvalue, json_type_int};
						json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &addscene);
						json_component data_Json = {"DATA",data_object,json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					else{
						jsonvalue = vrts_Json_String.delscene;
						json_component cmd = {"CMD","DELSCENE", json_type_string};
						json_component delscene = {"SCENEID",jsonvalue,json_type_int};
						json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &delscene);
						json_component data_Json = {"DATA", data_object, json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					break;
				case SCENE_STATUS:
					if(1){
						if(vrui_GWIF_LengthMeassge == 13){
							jsonvalue = vrts_GWIF_IncomeMessage->Message[9] | vrts_GWIF_IncomeMessage->Message[10]<<8;
						}
						else{
							jsonvalue = vrts_GWIF_IncomeMessage->Message[7] | vrts_GWIF_IncomeMessage->Message[8]<<8;
						}
						json_component cmd = {"CMD","CALLSCENE", json_type_string};
						json_component callscene = {"SCENEID",jsonvalue,json_type_int};
						json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &adr, &callscene);
						json_component data_Json = {"DATA",data_object, json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
					}
					break;
				case NODE_RESET_STATUS:
					if(1){
						json_component cmd = {"CMD","RESETNODE",json_type_string};
						json_object *data_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &adr);
						json_component data_Json = {"DATA",data_object, json_type_object};
						create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
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
				if(jsonAdr == unicastId){
					//pthread_mutex_lock(&vrpth_SHAREMESS_FlagCheckRsp);
					flag_check_rsp = true;
					puts("RSP OF opcode vendor");
					//pthread_mutex_unlock(&vrpth_SHAREMESS_FlagCheckRsp);
				}

				uint16_t jsonAdrgw= vrts_GWIF_IncomeMessage->Message[3] | (vrts_GWIF_IncomeMessage->Message[4]<<8);
				uint8_t opcodevendor = vrts_GWIF_IncomeMessage->Message[5];
				uint16_t vendorid = (vrts_GWIF_IncomeMessage->Message[6])|(vrts_GWIF_IncomeMessage->Message[7]<<8);
				uint16_t header = (vrts_GWIF_IncomeMessage->Message[8])|(vrts_GWIF_IncomeMessage->Message[9]<<8);
				json_component device_unicast_id_json = {"DEVICE_UNICAST_ID",jsonAdr,json_type_int};

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
							json_component json_id = {"DEVICE_ID",uuid,json_type_string};
							json_component json_device_key = {"DEVICE_KEY",device_key,json_type_string};
							json_component json_net_key = {"NET_KEY",net_key,json_type_string};
							json_component json_app_key = {"APP_KEY",app_key,json_type_string};
							json_component json_type_id = {"DEVICE_TYPE_ID",TypeConvertID(jsonType,jsonAttrubute,jsonApplication),json_type_int};
							json_object *data = create_json_obj_from(add_component_to_obj, 6,mqtt_dont_push, &device_unicast_id_json, &json_id, &json_device_key, &json_net_key, &json_app_key, &json_type_id);
							json_component jsondata = {"DATA",data,json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &jsondata);
							if(TypeConvertID(jsonType, jsonAttrubute, jsonApplication) == 23003){
								Function_Vendor(HCI_CMD_GATEWAY_CMD, SendTimeForScreenT_vendor_typedef, 65535, NULL16, NULL8, NULL8, NULL8, NULL16, NULL16, \
										NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, dataTimeInternet[0], dataTimeInternet[1], NULL8, NULL16, 23);
							}
						}
					}
					else if(header == HEADER_TYPE_SAVEGW){
						if(flag_saveGW){
							flag_saveGW = false;
							flag_checkSaveGW = true;
							json_component cmd = {"CMD","SAVEGATEWAY", json_type_string};
							json_component savegateway = {"ADR_GW",jsonAdrgw,json_type_int};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &savegateway);
							json_component data_Json = {"DATA", data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &data_Json);
						}
					}
				}
				if((opcodevendor == RD_OPCODE_SCENE_RSP) && (vendorid == VENDOR_ID))
				{
					uint8_t rspButtonId		= vrts_GWIF_IncomeMessage->Message[10];
					uint8_t rspModeId		= vrts_GWIF_IncomeMessage->Message[11];
					uint16_t rspSceneRgb 	= vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8);
					uint16_t rspSceneRemote = vrts_GWIF_IncomeMessage->Message[12] | (vrts_GWIF_IncomeMessage->Message[13]<<8);
					uint16_t rspSceneSensor = vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8);
					uint8_t rspSTT 			= vrts_GWIF_IncomeMessage->Message[10];
					uint16_t rspCondition 	= (vrts_GWIF_IncomeMessage->Message[12]<<8) | vrts_GWIF_IncomeMessage->Message[13];
					uint16_t rspLowLux		= vrts_GWIF_IncomeMessage->Message[14] | (vrts_GWIF_IncomeMessage->Message[15]<<8);
					uint16_t rspHighLux		= vrts_GWIF_IncomeMessage->Message[16] | (vrts_GWIF_IncomeMessage->Message[17]<<8);
					uint16_t rspActiontime 	= vrts_GWIF_IncomeMessage->Message[17] | (vrts_GWIF_IncomeMessage->Message[18]<<8);
					uint8_t rspSrgbIdRemote	= vrts_GWIF_IncomeMessage->Message[16];
					uint8_t rspSrgbIdSensor	= vrts_GWIF_IncomeMessage->Message[18];
					uint8_t modeRgb			= vrts_GWIF_IncomeMessage->Message[12];

					json_component srgbid_Json 	= {"SRGBID",modeRgb,json_type_int};
					json_component sceneid_Json = {"SCENEID",rspSceneRgb,json_type_int};
//					json_component delscenergb 	= {"SCENEID",rspSceneRgb,json_type_int};
//					json_component setscenergb 	= {"SCENEID",rspSceneRgb,json_type_int};
//					json_component callscenergb = {"SCENEID",rspSceneRgb,json_type_int};

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
						if(1){
							json_component cmd = {"CMD", "CALLMODE_RGB", json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &srgbid_Json);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_DEL:
						if(1){
							json_component cmd = {"CMD","DELSCENE_RGB", json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &sceneid_Json);
							json_component data_Json = {"DATA", data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_SET:
						if(1){
							json_component cmd = {"CMD", "ADDSCENE_RGB", json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &device_unicast_id_json, &srgbid_Json, &sceneid_Json);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_CALL_SCENE_RGB:
						if(1){
							json_component cmd = {"CMD","CALLSCENE_RGB", json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &sceneid_Json);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_REMOTE_DC_SET:
						if(1){
							json_component cmd = {"CMD","ADDSCENE_REMOTE_DC",json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &device_unicast_id_json, &btid, &modid, &sceneidR);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push,&cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_REMOTE_DC_DEL:
						if(1){
							json_component cmd = {"CMD","DELSCENE_REMOTE_DC",json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &device_unicast_id_json, &btid, &modid);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_REMOTE_AC_SET:
						if(1){
							json_component cmd = {"CMD","ADDSCENE_REMOTE_AC",json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &device_unicast_id_json, &btid, &modid, &sceneidR);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2,mqtt_push,&cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_REMOTE_AC_DEL:
						if(1){
							json_component cmd = {"CMD","DELSCENE_REMOTE_AC",json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &device_unicast_id_json, &btid, &modid);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_LIGHT_PIR_SET:
						if(1){
							json_component cmd = {"CMD","ADDSCENE_LIGHT_PIR_SENSOR",json_type_string};
							RD_Sensor_data_tdef data_Scene_Rsp;
							data_Scene_Rsp.data = (vrts_GWIF_IncomeMessage->Message[12]<<24)|(vrts_GWIF_IncomeMessage->Message[13]<<16)|(vrts_GWIF_IncomeMessage->Message[14]<<8);
							json_component pir_json = {"PIR",data_Scene_Rsp.Pir_Conditon,json_type_int};
							json_object *pir_Object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &pir_json);
							json_component pir_Object_Json = {"PIR_SENSOR",pir_Object,json_type_object};
							printf("condition light= %d\n",data_Scene_Rsp.Light_Conditon);
							if(data_Scene_Rsp.Light_Conditon == 0){
								char type = 1;
								json_component type_json = {"TYPE",type,json_type_int};
								json_object * data_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &device_unicast_id_json, &type_json, &pir_Object_Json, &sceneid_Json);
								json_component data_json = {"DATA",data_object,json_type_object};
								create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_json);
							}
							else {
								char type = 2;
								json_component type_json = {"TYPE",type,json_type_int};
								if(data_Scene_Rsp.Light_Conditon == 1){
									json_component condition_Light_Json = {"CONDITION",4,json_type_int};
									json_component low_lux_Json = {"LOW_LUX",data_Scene_Rsp.Lux_low*10,json_type_int};
									json_object * light_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &condition_Light_Json, &low_lux_Json);
									json_component light_json = {"LIGHT_SENSOR",light_object, json_type_object};
									json_object *data_object = create_json_obj_from(add_component_to_obj, 5, mqtt_dont_push, & device_unicast_id_json, &type_json, &light_json, &pir_Object_Json, &sceneid_Json);
									json_component data_json = {"DATA",data_object, json_type_object};
									create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_json);
								}
								else if(data_Scene_Rsp.Light_Conditon == 2){
									json_component condition_Light_Json = {"CONDITION",7,json_type_int};
									json_component low_lux_Json = {"LOW_LUX",data_Scene_Rsp.Lux_low*10,json_type_int};
									json_component high_lux_Json = {"HIGHT_LUX",data_Scene_Rsp.Lux_hi*10, json_type_int};
									json_object * light_object = create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &condition_Light_Json, &low_lux_Json,&high_lux_Json);
									json_component light_json = {"LIGHT_SENSOR",light_object, json_type_object};
									json_object *data_object = create_json_obj_from(add_component_to_obj, 5, mqtt_dont_push, & device_unicast_id_json, &type_json, &light_json, &pir_Object_Json, &sceneid_Json);
									json_component data_json = {"DATA",data_object, json_type_object};
									create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_json);
								}
								else if(data_Scene_Rsp.Light_Conditon == 3){
									json_component condition_Light_Json = {"CONDITION",6,json_type_int};
									json_component low_lux_Json = {"LOW_LUX",data_Scene_Rsp.Lux_low*10,json_type_int};
									json_object * light_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &condition_Light_Json, &low_lux_Json);
									json_component light_json = {"LIGHT_SENSOR",light_object, json_type_object};
									json_object *data_object = create_json_obj_from(add_component_to_obj, 5, mqtt_dont_push, & device_unicast_id_json, &type_json, &light_json, &pir_Object_Json, &sceneid_Json);
									json_component data_json = {"DATA",data_object, json_type_object};
									create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_json);
								}
							}
						}
						break;
					case HEADER_SCENE_LIGHT_PIR_DEL:
						if(1){
							json_component cmd = {"CMD","DELSCENE_LIGHT_PIR_SENSOR",json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &jsonSceneDel);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_DOOR_SENSOR_SET:
						if(1){
							json_component cmd = {"CMD","ADDSCENE_DOOR_SENSOR",json_type_string};
							json_component door_value = {"DOOR",vrts_GWIF_IncomeMessage->Message[12],json_type_int};
							json_object * door_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push,&door_value);
							json_component door_json = {"DOOR_SENSOR",door_object,json_type_object};
							json_component sceneid_json = {"SCENEID",(vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8)),json_type_int};
							json_component srgbid_json = {"SRGBID",vrts_GWIF_IncomeMessage->Message[13],json_type_int};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &device_unicast_id_json, &door_json, &sceneid_json, &srgbid_json);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push,&cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_DOOR_SENSOR_DEL:
						if(1){
							json_component cmd = {"CMD","DELSCENE_DOOR_SENSOR",json_type_string};
							json_component sceneid_json = {"SCENEID",(vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8)),json_type_int};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &sceneid_json);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_CONTROL_SWITCH4:
						if(1){
							json_component cmd = {"CMD", "CONTROL_SWITCH4", json_type_string};
							uint8_t numRelay_Rsp = vrts_GWIF_IncomeMessage->Message[10];
							uint8_t valRelay_Rsp = vrts_GWIF_IncomeMessage->Message[11];
							if(numRelay_Rsp ==1){
								json_component relay_json = {"RELAY1",valRelay_Rsp, json_type_int};
								json_object *switch4_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &relay_json);
								json_component switch4_json = {"SWITCH_STATUS",switch4_object, json_type_object};
								json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &switch4_json);
								json_component data_Json = {"DATA", data_object, json_type_object};
								create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
							}
							else if(numRelay_Rsp ==2){
								json_component relay_json = {"RELAY2",valRelay_Rsp, json_type_int};
								json_object *switch4_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &relay_json);
								json_component switch4_json = {"SWITCH_STATUS",switch4_object, json_type_object};
								json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &switch4_json);
								json_component data_Json = {"DATA", data_object, json_type_object};
								create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
							}
							else if(numRelay_Rsp ==3){
								json_component relay_json = {"RELAY3",valRelay_Rsp, json_type_int};
								json_object *switch4_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &relay_json);
								json_component switch4_json = {"SWITCH_STATUS",switch4_object, json_type_object};
								json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &switch4_json);
								json_component data_Json = {"DATA", data_object, json_type_object};
								create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
							}
							else if(numRelay_Rsp ==4){
								json_component relay_json = {"RELAY4",valRelay_Rsp, json_type_int};
								json_object *switch4_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &relay_json);
								json_component switch4_json = {"SWITCH_STATUS",switch4_object, json_type_object};
								json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &switch4_json);
								json_component data_Json = {"DATA", data_object, json_type_object};
								create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
							}
						}
						break;
					case HEADER_SCENE_SWITCH4_SET:
						if(1){
							json_component cmd = {"CMD","ADDSCENE_SWITCH4", json_type_string};
							json_component sceneId_json = {"SCENEID",vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8), json_type_int} ;
							uint8_t relay1 = vrts_GWIF_IncomeMessage->Message[12];
							uint8_t relay2 = vrts_GWIF_IncomeMessage->Message[13];
							uint8_t relay3 = vrts_GWIF_IncomeMessage->Message[14];
							uint8_t relay4 = vrts_GWIF_IncomeMessage->Message[15];
							json_component relay1_json = {"RELAY1",relay1, json_type_int};
							json_component relay2_json = {"RELAY2",relay2, json_type_int};
							json_component relay3_json = {"RELAY3",relay3, json_type_int};
							json_component relay4_json = {"RELAY4",relay4, json_type_int};
							json_object *switch4_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &relay1_json, &relay2_json, &relay3_json, &relay4_json);
							json_component switch4_json = {"SWITCH_STATUS",switch4_object, json_type_object};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 3, mqtt_dont_push, &device_unicast_id_json, &switch4_json, &sceneId_json);
							json_component data_Json = {"DATA", data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_SWITCH4_DEL:
						if(1){
							json_component cmd = {"CMD","DELSCENE_SWITCH4", json_type_string};
							json_component sceneId_json = {"SCENEID",vrts_GWIF_IncomeMessage->Message[10] | (vrts_GWIF_IncomeMessage->Message[11]<<8), json_type_int} ;
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &sceneId_json);
							json_component data_Json = {"DATA", data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_LIGHT_SENSOR_SET_BALANCE:
						if(1){
							uint16_t lightness = (vrts_GWIF_IncomeMessage->Message[10]) | (vrts_GWIF_IncomeMessage->Message[11]<<8);
							uint16_t destinantion = (vrts_GWIF_IncomeMessage->Message[12]) | (vrts_GWIF_IncomeMessage->Message[13]<<8);
							json_component cmd = {"CMD","SET_BALANCE_LIGHT_SENSOR",json_type_string};
							json_component lightness_json = {"LIGHTNESS", lightness, json_type_int};
							json_component destination_json = {"DESTINATION", destinantion, json_type_int};
							json_object *light_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &lightness_json, &destination_json);
							json_component light_json = {"LIGHT_SENSOR",light_object, json_type_object};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &light_json);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_LIGHT_SENSOR_DEL:
						if(1){
							json_component cmd = {"CMD","DEL_BALANCE_LIGHT_SENSOR",json_type_string};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 1, mqtt_dont_push, &device_unicast_id_json);
							json_component data_Json = {"DATA",data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_SCREENT_SET:
						if(1){
							uint8_t button_screen 	= vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
							uint16_t sceneID_screen = vrts_GWIF_IncomeMessage->Message[11] | (vrts_GWIF_IncomeMessage->Message[12]<<8);
							uint8_t srgbid_screen  	= vrts_GWIF_IncomeMessage->Message[13];
							json_component cmd = {"CMD","ADDSCENE_SCREEN_TOUCH", json_type_string};
							json_component button_json = {"BUTTONID", button_screen, json_type_int};
							json_component sceneid_json = {"SCENEID", sceneID_screen, json_type_int};
							json_component srgb_json = {"SRGBID", srgbid_screen , json_type_int};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 4, mqtt_dont_push, &device_unicast_id_json, &button_json, &sceneid_Json, &srgb_json);
							json_component data_Json = {"DATA", data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
						}
						break;
					case HEADER_SCENE_SCREENT_DEL:
						if(1){
							uint8_t button_screen 	= vrts_GWIF_IncomeMessage->Message[10] & 0xFF;
							json_component cmd = {"CMD","DELSCENE_SCREEN_TOUCH", json_type_string};
							json_component button_json = {"BUTTONID", button_screen, json_type_int};
							json_object *data_object = create_json_obj_from(add_component_to_obj, 2, mqtt_dont_push, &device_unicast_id_json, &button_json);
							json_component data_Json = {"DATA", data_object, json_type_object};
							create_json_obj_from(add_component_to_obj, 2, mqtt_push, &cmd, &data_Json);
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
