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


#define mqtt_host 			"192.168.100.1"
#define mqtt_port 			1883
#define mqtt_username 		"RD"
#define mqtt_password 		"1"




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
