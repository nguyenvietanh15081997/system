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
#include <math.h>

#include <json-c/json.h>
#include <mosquitto.h>


#define mqtt_host 			"postman.cloudmqtt.com"
#define mqtt_port 			13001
#define mqtt_username 		"insklndl"
#define mqtt_password 		"JCCSw9dYngMF"

#define TP_PROVISION_START            "RD/PROVISION/START"
#define TP_PROVISION_STOP             "RD/PROVISION/STOP"
#define TP_CONTROL_UPDATE		"RD/CONTROL/UPDATE"
#define TP_CONTROL_ONOFF        "RD/CONTROL/ONOFF"
#define TP_CONTROL_CCT        	"RD/CONTROL/CCT"
#define TP_CONTROL_DIM   		"RD/CONTROL/DIM"
#define TP_CONTROL_HSL   		"RD/CONTROL/HSL"
#define TP_CONTROL_ADDGROUP   	"RD/CONTROL/ADDGROUP"
#define TP_CONTROL_DELGROUP   	"RD/CONTROL/DELGROUP"
#define TP_CONTROL_ADDSENCE   	"RD/CONTROL/ADDSENCE"
#define TP_CONTROL_DELSENCE   	"RD/CONTROL/DELSENCE"
#define TP_CONTROL_RESETNODE   	"RD/CONTROL/RESETNODE"

#define TP_STATUS_ONOFF   	"RD/STATUS/ONOFF"
#define TP_STATUS_CCT   	"RD/STATUS/CCT"
#define TP_STATUS_DIM   	"RD/STATUS/DIM"
#define TP_STATUS_HSL   	"RD/STATUS/HSL"
#define TP_STATUS_ADDGROUP   "RD/STATUS/ADDGROUP"
#define TP_STATUS_DELGROUP   "RD/STATUS/DELGROUP"
#define TP_STATUS_ADDSENCE   "RD/STATUS/ADDSENCE"
#define TP_STATUS_DELSENCE   "RD/STATUS/DELSENCE"

extern int run;

extern bool flag_on_all;
extern bool flag_off_all;

void handle_signal(int s);
void connect_callback(struct mosquitto *mosq, void *obj, int result);
//void json_parse(json_object * jobj);
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
int mqtt_send(struct mosquitto *mosq, char *msg);
void * MQTT_Thread(void *argv);

#ifdef __cplusplus
}
#endif

#endif /* GATEWAYMANAGER_MQTT_H_ */
