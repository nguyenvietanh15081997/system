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

pthread_t vrts_System_TestSend;

int run = 1;
void handle_signal(int s)
{
	run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	_Bool match = 0;
	//int msgled = message->payloadlen;
	char* msg = (char*)message->payload;
	char on[3] = "on";
	char off[4] = "off";
	char scan[5] = "scan";
	char stop[5] = "stop";
	char timepoll3s[7] = "poll5s";
	char timepoll1s[8] = "poll1s";

	if(strcmp(msg, scan)==0){
		puts("Provision start");
		MODE_PROVISION=true;
		pthread_create(&vrts_System_TestSend,NULL, ProvisionThread, NULL);
	}
	if(strcmp(msg, stop)==0){
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
	if(strcmp(msg, on) == 0){
		puts("On");
		FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,0xffff,0, 1, 0, 14);
	}
	if(strcmp(msg,off) == 0){
		puts("Off");
		FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,0xffff,0, 0, 0, 14);
	}
	if(strcmp(msg,timepoll3s) == 0){
		puts("poll 5s");
		FunctionPer(HCI_CMD_GATEWAY_CMD, SetTimePoll_typedef, 0x0013, 0, 0, 5000,15);
	}
	if(strcmp(msg,timepoll1s) == 0){
		puts("poll 1s");
		FunctionPer(HCI_CMD_GATEWAY_CMD, SetTimePoll_typedef, 0x0013, 0, 0, 1000,15);
	}

	//puts(msg);

	//printf("message '%.*s' for topic '%s'\n", message->payloadlen, msg, message->topic);
	mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);
	if (match) {
		printf("got message for ADC topic\n");
	}
}

int mqtt_send(struct mosquitto *mosq, char *msg)
{
	mosquitto_publish(mosq, NULL, "/testtopic", strlen(msg), msg, 0, 0);
	return 0;
}
void * MQTT_Thread(void *argv)
{
		char clientid[24];
		struct mosquitto *mosq;
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

			mosquitto_subscribe(mosq, NULL, "Topic_RangDong", 0);
			int snd = mqtt_send(mosq, "Rang Dong");
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

