
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


#define mqtt_host 			"postman.cloudmqtt.com"//"soldier.cloudmqtt.com"//"192.168.33.1"//"192.168.100.1"//
#define mqtt_port 			13001//11875//1883//
#define mqtt_username 		"insklndl"//"jpcvzwgj"//"RD"//
#define mqtt_password 		"x9aBwks70kmQ"//"JCCSw9dYngMF"//"2k756Wus2bJE"//"1"//



#define TP_STATUS            "RD_STATUS"

extern int run;


extern struct mosquitto *mosq;

void handle_signal(int s);
void connect_callback(struct mosquitto *mosq, void *obj, int result);
//void json_parse(json_object * jobj);
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
int mqtt_send(struct mosquitto *mosq, char * topic,char *msg);
void * MQTT_Thread(void *argv);

#ifdef __cplusplus
}
#endif

#endif /* GATEWAYMANAGER_MQTT_H_ */
