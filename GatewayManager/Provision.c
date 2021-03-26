/*
 *Provision.c
 */

#include "../GatewayManager/Provision.h"
#include "../GatewayManager/LedProcess.h"
#include "../GatewayManager/slog.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/LedProcess.h"
#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/MQTT.h"

pthread_t tmp;
pthread_t vrts_System_Gpio;

unsigned int Timeout_CheckDataBuffer = 0;
unsigned char scanNotFoundDev =0 ;
unsigned int adr_heartbeat;

uint8_t OUTMESSAGE_ScanStop[3]     = {0xE9, 0xFF, 0x01};
uint8_t OUTMESSAGE_ScanStart[3]    = {0xE9, 0xFF, 0x00};
uint8_t OUTMESSAGE_MACSelect[9]    = {0xE9, 0xFF, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t OUTMESSAGE_GetPro[3] 	   = {0xE9, 0xFF, 0x0C};
uint8_t OUTMESSAGE_Provision[28]   = {0};
uint8_t OUTMESSAGE_BindingALl[22]  = {0xe9, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x60, 0x96, 0x47, 0x71, 0x73, 0x4f, 0xbd, 0x76, 0xe3, 0xb4, 0x05, 0x19, 0xd1, 0xd9, 0x4a, 0x48};

uint8_t setpro_internal[]       =   {0xe9, 0xff, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x01, 0x00};
uint8_t admit_pro_internal[]    =   {0xe9, 0xff, 0x0d, 0x01, 0x00, 0xff, 0xfb, 0xeb, 0xbf, 0xea, 0x06, 0x09, 0x00, 0x52, 0x90, 0x49, 0xf1, 0xf1, 0xbb, 0xe9, 0xeb};// trả về unicast tiếp theo của con đèn cần thêm vào
uint8_t device_key1[50];

bool flag_selectmac     	= false;
bool flag_getpro_info   	= false;
bool flag_getpro_element	= false;
bool flag_provision     	= false;
bool flag_mac           	= true;
bool flag_check_select_mac  = false;
bool flag_done          	= true;
bool flag_setpro            = false;
bool flag_admitpro          = false;
bool flag_checkadmitpro     = true;

bool flag_set_type          = false;

void ControlMessage(uint16_t lengthmessage,uint8_t *message)
{
	pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
	vrb_SHAREMESS_Send2GatewayAvailabe = true;
	vrui_SHAREMESS_Send2GatewayLength = lengthmessage;
	memcpy(vrsc_SHAREMESS_Send2GatewayMessage, message, vrui_SHAREMESS_Send2GatewayLength);
	pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);

	/*slog*/
//	uint8_t *tempDataUart;
	uint8_t tempDataLog[200]="";
	uint8_t temp[4];
//
//	tempDataUart = (uint8_t *)mes;
	int i;
	//strcpy(TempData2,h);
	for(i=0;i< lengthmessage;i++){
		sprintf(temp,"%x ",message[i]);
		strcat(tempDataLog,temp);
	}
	slog_info("(cmd)%s",tempDataLog);
}
void *ProvisionThread (void *argv )
{
	tmp = pthread_self();
	while(MODE_PROVISION){
		if((flag_done == true) || (Timeout_CheckDataBuffer == 20000))
		{
			scanNotFoundDev++;
			if(scanNotFoundDev==3)
			{
				scanNotFoundDev = 0;
				MODE_PROVISION=false;
				ControlMessage(3, OUTMESSAGE_ScanStop);
				slog_print(SLOG_INFO, 1, "<provision>Provision stop");

				struct json_object *object;
				object = json_object_new_object();
				json_object_object_add(object, "CMD", json_object_new_string("STOP"));
				char *rsp;
				rsp = json_object_to_json_string(object);
				mosquitto_publish(mosq, NULL, "RD_STATUS", strlen(rsp),rsp,  0, 0);
				slog_info("(mqtt)Message_send:%s",rsp);

				flag_selectmac     = false;
				flag_getpro_info   = false;
				flag_getpro_element= false;
				flag_provision     = false;
				flag_mac           = true;
				flag_check_select_mac  = false;
				flag_done          = true;
				flag_setpro  = false;
				flag_admitpro = false;
				flag_checkadmitpro = true;
				flag_set_type = false;

				// for gpio
				flag_close_gpio = false;
				flag_blink = false;
				led_pin_off(gpio[LED_BLE_PIN_INDEX]);
				pthread_cancel(tmp1);
			}
			else
			{
				flag_done = false;
				Timeout_CheckDataBuffer=0;
				//usleep(500000);
				ControlMessage(3, OUTMESSAGE_ScanStart);
				slog_print(SLOG_INFO, 1, "<provision>SCAN");
				flag_check_select_mac= true;

				// for gpio
				pthread_create(&tmp1,NULL, Led_Thread, NULL);
				flag_blink = true;
				flag_close_gpio = false;
			}
		}

		if((flag_selectmac==true) && (flag_mac==true))
		{
			flag_selectmac=false;
			flag_mac=false;
			ControlMessage(9, OUTMESSAGE_MACSelect);
			slog_print(SLOG_INFO, 1, "<provision>SELECTMAC");
			usleep(100000);
			ControlMessage(3, OUTMESSAGE_GetPro);
			slog_print(SLOG_INFO, 1, "<provision>GETPRO");
			flag_checkadmitpro = false;
			//sleep(3);
		}
		if(flag_setpro == true)
		{
			flag_setpro = false;
			srand((int)time(0));
			int random;
			int i;
			for(i=0;i<16;i++)
			{
				random=rand()%256;
				setpro_internal[i+3]=random;
			}
			slog_print(SLOG_INFO, 1, "<provision>SETPRO....");
			ControlMessage(28, setpro_internal);
			slog_print(SLOG_INFO, 1, "<provision>ADMITPRO...");
			ControlMessage(21, admit_pro_internal);
			/*add device key to file*/
			sprintf(device_key1,"%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",admit_pro_internal[5],admit_pro_internal[6],admit_pro_internal[7],\
					admit_pro_internal[8],admit_pro_internal[9],admit_pro_internal[10],admit_pro_internal[11],admit_pro_internal[12],admit_pro_internal[13],\
					admit_pro_internal[14],admit_pro_internal[15],admit_pro_internal[16],admit_pro_internal[17], admit_pro_internal[18],\
					admit_pro_internal[19],admit_pro_internal[20]);
			FILE *file=fopen("/root/device_key.txt","w");
			   if(file == NULL)
			   {
			      printf("Error!");
			      exit(1);
			   }
			   fprintf(file,"%s",device_key1);
			   fclose(file);
			/**/
			//usleep(100000);
			flag_checkadmitpro = true;

		}
		if(flag_admitpro == true && flag_checkadmitpro== true)
		{
			flag_admitpro = false;
			flag_checkadmitpro = false;
			slog_print(SLOG_INFO, 1, "<provision>GETPRO");
			ControlMessage(3, OUTMESSAGE_GetPro);
		}
		if((flag_getpro_info == true) && (flag_getpro_element == true))
		{
			flag_getpro_info = false;
			flag_getpro_element = false;
			ControlMessage(28, OUTMESSAGE_Provision);
			slog_print(SLOG_INFO, 1, "<provision>PROVISION");
		}
		if(flag_provision == true)
		{
			flag_provision = false;
			//sleep(1);
			ControlMessage(22, OUTMESSAGE_BindingALl);
			slog_print(SLOG_INFO, 1, "<provision>BINDING ALL");
			flag_set_type = false;
		}
		if(flag_set_type == true)
		{
			flag_set_type = false;
//			HeartBeat(HCI_CMD_GATEWAY_CMD, adr_heartbeat, 1, 255, 11, 5, 7, 21);
//			usleep(400000);
//			 Function_Vendor(HCI_CMD_GATEWAY_CMD, SaveGateway_vendor_typedef, adr_heartbeat, NULL16,\
//					 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,17);
//			 usleep(400000);
//			 Function_Vendor(HCI_CMD_GATEWAY_CMD, AskTypeDevice_vendor_typedef, adr_heartbeat, NULL16,\
//					 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,17);
//			 usleep(400000);
			flag_done=true;
			flag_mac=true;
		}

	}
	return NULL;
}
