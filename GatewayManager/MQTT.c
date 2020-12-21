/*
 * MQTT.c
 *
 *  Created on: Nov 6, 2020
 *      Author: rd
 */


#include "../GatewayManager/MQTT.h"
#include "../GatewayManager/OpCode.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/JsonProcess.h"

pthread_t vrts_System_TestSend;
struct mosquitto *mosq;
int run = 1;
void handle_signal(int s)
{
	run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect callback, rc=%d\n", result);
}
/*
 * TODO: check json string and process
 */
void CheckTopic(char *topicJson, uint16_t adr, uint16_t par)
{
	if(strcmp(topicJson,TP_PROVISION_START)==0){
		puts("Provision start");
		MODE_PROVISION=true;
		pthread_create(&vrts_System_TestSend,NULL, ProvisionThread, NULL);
	}
	else if(strcmp(topicJson,TP_PROVISION_STOP)==0){
		puts("Provision stop");
		MODE_PROVISION=false;
		pthread_cancel(tmp);
		ControlMessage(3, OUTMESSAGE_ScanStop);
		flag_selectmac     = false;
		flag_getpro_info   = false;
		flag_getpro_element= false;
		flag_provision     = false;
		flag_mac           = true;
		flag_check_select_mac  = false;
		flag_done          = true;
	}
	else if(strcmp(topicJson,TP_CONTROL_ONOFF)==0){
		if(par){
			puts("On");
			FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,adr,NULL8, par, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
		}
		else if(!par){
			puts("Off");
			FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,adr,NULL8, par, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
		}
	}
	else if(strcmp(topicJson,TP_CONTROL_CCT) == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_typedef, adr, NULL8, NULL8, NULL16,par, NULL16, NULL16,NULL16, NULL16, NULL16, 17);
	}
	else if(strcmp(topicJson,TP_CONTROL_DIM) == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, Lightness_Set_typedef, adr, NULL8, NULL8, par, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
	}
	else if(strcmp(topicJson,TP_CONTROL_UPDATE) == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, UpdateLight_typedef, adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
	}
	else if(strcmp(topicJson,TP_CONTROL_ADDGROUP) == 0){
		check_add_or_del_group= true;
		FunctionPer(HCI_CMD_GATEWAY_CMD, AddGroup_typedef, adr, par, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
	}
	else if(strcmp(topicJson,TP_CONTROL_DELGROUP) == 0){
		check_add_or_del_group= false;
		FunctionPer(HCI_CMD_GATEWAY_CMD, DelGroup_typedef, adr, par, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
	}
	else if(strcmp(topicJson,TP_CONTROL_ADDSENCE) == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef, adr, NULL8, NULL8, NULL16, NULL16, par, NULL16,NULL16, NULL16, NULL16, 14);
	}
	else if(strcmp(topicJson,TP_CONTROL_CALLSENCE) == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16, adr, NULL16,NULL16, NULL16, NULL16, 17);
	}
	else if(strcmp(topicJson,TP_CONTROL_DELSENCE) == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, adr, NULL8, NULL8, NULL16, NULL16, par, NULL16,NULL16, NULL16, NULL16, 14);
	}
	else if(strcmp(topicJson,TP_CONTROL_RESETNODE) == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, ResetNode_typedef, adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
	}
	//else if(strcmp(topic))
}
//void FunctionPer(uint16_t cmd,\
//				functionTypeDef Func,\
//				uint16_t unicastAdr,\
//				uint8_t adrGroup,\
//				uint8_t parStatusOnOff,\
//				uint16_t parLightness,\
//				uint16_t parCCT,\
//				uint16_t parSenceId,\
//				uint16_t parTimePoll,\
//				uint8_t cmdLenght);
//void ProcessJson()
//{
//	switch (flagDefineCmd){
//	case
//	}
//}
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	//struct json_obj* jobj;
	char* msg = (char*)message->payload;
	int i;
	numObject=0;
	for(i=0;i<strlen(msg);i++){
		if(msg[i]==':') {numObject++;}
	}
	//printf ("%d",numObject);
	struct json_obj* jobj = json_tokener_parse(msg);
	Json_Parse(jobj);
	//CheckTopic(message->topic,valueJsonRec[0],valueJsonRec[1]);




//	mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);
//	if (match) {
//		printf("got message for ADC topic\n");
//	}
}

int mqtt_send(struct mosquitto *mosq, char *msg)
{
	mosquitto_publish(mosq, NULL, "RD/STATUS/SYSMSG", strlen(msg), msg, 0, 0);
	return 0;
}
void * MQTT_Thread(void *argv)
{
		char clientid[24];
		//struct mosquitto *mosq;
		int rc = 0;
		int abc = 0;

		signal(SIGINT, handle_signal);
		signal(SIGTERM, handle_signal);

		mosquitto_lib_init();

		memset(clientid, 0, 24);
		snprintf(clientid, 23, "mysql_log_%d", getpid());
		mosq = mosquitto_new(clientid, true, 0);
		if(mosq){
			mosquitto_connect_callback_set(mosq, connect_callback);
			mosquitto_message_callback_set(mosq, message_callback);

			abc = mosquitto_username_pw_set(mosq, mqtt_username, mqtt_password);
			rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);

			mosquitto_subscribe(mosq,NULL, "RD/PROVISION/+",0);
			mosquitto_subscribe(mosq,NULL, "RD/CONTROL/+",0);

			int snd = mqtt_send(mosq, "{\"MSG\":\"\"RANG DONG ANH HUNG VA CO BAC HO\"}");
			if(snd != 0) printf("mqtt_send error=%i\n", snd);
			sleep(10);

			while(run){
				rc= abc = mosquitto_loop(mosq, -1, 1);
				if(run && rc){
					printf("connection error!\n");
					sleep(10);
					mosquitto_reconnect(mosq);
				}
			}
			mosquitto_destroy(mosq);
		}
		mosquitto_lib_cleanup();
		return NULL;
}

