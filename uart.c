/*
 ============================================================================
 Name        : GatewayManager.c
 Author      : haha
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "GatewayManager/GateInterface.h"
#include "GatewayManager/MQTT.h"

pthread_t vrts_System_ThreadGWIF;
pthread_t vrts_System_TestSend;
//pthread_t vrts_System_MQTT;


static uint8_t OUTMESSAGE_ScanStart[3] = {0xE9, 0xFF, 0x00};

void controlmessage(uint16_t lengthmessage,uint8_t *Message)
{
	pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
	vrb_SHAREMESS_Send2GatewayAvailabe = true;
	vrui_SHAREMESS_Send2GatewayLength = lengthmessage;
	memcpy(vrsc_SHAREMESS_Send2GatewayMessage, Message, vrui_SHAREMESS_Send2GatewayLength);
	pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
}


/*
 * Ham thu nghiem Scan Network de xu ly ban tin ve tu Gateway
 * TODO: Hoan thien va loai bo
 */

void *TestSendThread (void *vargp){
while(1)
{

	if((flag_done == true) || (timeout == 32000))
	{
		//printf("flag_done: %d\n",flag_done);
		flag_done = false;
		//flag_scan=false;
		timeout=0;
		sleep(1);
		controlmessage(3, OUTMESSAGE_ScanStart);
		printf ("scan\n");
		//printf("flag_done: %d\n",flag_done);
		flag_check_select_mac= true;
	}

	if((flag_selectmac==true) && (flag_mac==true) )
	{
		//printf("flag_selectmac: %d\n",flag_selectmac);
		//printf("flag_mac: %d\n",flag_mac);
		flag_selectmac=false;
		flag_mac=false;
        controlmessage(9, OUTMESSAGE_MACSelect);
		printf("selectmac\n");
		sleep(2);

		controlmessage(3, OUTMESSAGE_GetPro);
		printf ("getpro\n");
		sleep(1);
	}
	if((flag_getpro_info == true) && (flag_getpro_element == true))
	{
		flag_getpro_info = false;
		flag_getpro_element = false;
		controlmessage(28, OUTMESSAGE_Provision);
		printf ("provision\n");
	}

	if(flag_provision == true)
	{
		flag_provision=false;
		sleep(1);
		controlmessage(22, OUTMESSAGE_BindingALl);
		printf ("binding all\n");
	}
}
	return NULL;
}
//void *MQTT_Thread(void *vargp)
//{
//	//while(1){
//		char clientid[24];
//		struct mosquitto *mosq;
//		int rc = 0;
//		int abc = 0;
//		//char buf = "vuhongtu";
//
//		signal(SIGINT, handle_signal);
//		signal(SIGTERM, handle_signal);
//
//		mosquitto_lib_init();
//
//		memset(clientid, 0, 24);
//		snprintf(clientid, 23, "mysql_log_%d", getpid());
//		mosq = mosquitto_new(clientid, true, 0);
//
//
//		while(1){
//		if(mosq){
//			mosquitto_connect_callback_set(mosq, connect_callback);
//			mosquitto_message_callback_set(mosq, message_callback);
//
//			abc = mosquitto_username_pw_set(mosq, mqtt_username, mqtt_password);
//			rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
//
//			mosquitto_subscribe(mosq, NULL, "abc", 0);
//			int snd = mqtt_send(mosq, "vuhongtu");
//			if(snd != 0) printf("mqtt_send error=%i\n", snd);
//			sleep(10);
//
//			while(run){
//				rc= abc = mosquitto_loop(mosq, -1, 1);
//				if(run && rc){
//					printf("connection error!\n");
//					sleep(10);
//					mosquitto_reconnect(mosq);
//				}
//			}
//			mosquitto_destroy(mosq);
//		}
//
//		mosquitto_lib_cleanup();
//
//		return rc;
//		}
//	//}
//    return NULL;
//}



/*z
 * Ham xu ly chinh. Thuc thi:
 * - Goi cac tac vu tuong ung cua chuong trinh
 * PENDING: Tiep tuc bo sung them cac Task khac
 */
int main(void) {

	puts("--------------Start system123-----------------");
	//flag_scan=true;
	flag_mac=true;
	flag_done=true;
    pthread_create(&vrts_System_ThreadGWIF, NULL, GWINF_Thread, NULL);
	pthread_create(&vrts_System_TestSend, NULL, TestSendThread, NULL);
	//pthread_create(&vrts_System_MQTT, NULL, MQTT_Thread, NULL);
	pthread_join(vrts_System_ThreadGWIF, NULL);
	pthread_join(vrts_System_TestSend, NULL);
	//pthread_join(vrts_System_MQTT, NULL);
	while(1){
	}
	return EXIT_SUCCESS;
}
