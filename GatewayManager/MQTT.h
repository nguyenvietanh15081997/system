/*
 * MQTT.h
 *
 *  Created on: Nov 6, 2020
 *      Author: rd
 */

#ifndef GATEWAYMANAGER_MQTT_H_
#define GATEWAYMANAGER_MQTT_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <json-c/json.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <mosquitto.h>


#define mqtt_host 			"postman.cloudmqtt.com"
#define mqtt_port 			13001
#define mqtt_username 		"insklndl"
#define mqtt_password 		"JCCSw9dYngMF"




extern int run;

extern bool flag_on_all;
extern bool flag_off_all;

void handle_signal(int s);
void connect_callback(struct mosquitto *mosq, void *obj, int result);
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
int mqtt_send(struct mosquitto *mosq, char *msg);
void * MQTT_Thread(void *argv);

#ifdef __cplusplus
}
#endif

#endif /* GATEWAYMANAGER_MQTT_H_ */
