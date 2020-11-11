/*
 * MQTT.c
 *
 *  Created on: Nov 6, 2020
 *      Author: rd
 */


#include "../GatewayManager/MQTT.h"


int run = 1;
bool flag_on_all = false;
bool flag_off_all = false;
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
	int msgled = message->payloadlen;
	char* msg = (char*)message->payload;
	char on[3] = "on";
	char off[4] = "off";
	if(strcmp(msg, on)==0)
	{
		puts("ON");
	}
	if(strcmp(msg, off)==0)
	{
		puts("OFF");
	}

	//printf("message '%.*s' for topic '%s'\n", message->payloadlen, msg, message->topic);
	mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);
	if (match) {
		printf("got message for ADC topic\n");
	}
}

int mqtt_send(struct mosquitto *mosq, char *msg)
{
	mosquitto_publish(mosq, NULL, "/testtopic", strlen(msg), msg, 0, 0);
}
