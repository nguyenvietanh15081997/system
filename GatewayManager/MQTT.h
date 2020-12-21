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


#define mqtt_host 			"postman.cloudmqtt.com"//"soldier.cloudmqtt.com"//"192.168.33.1"//
#define mqtt_port 			13001//11875//1883//
#define mqtt_username 		"insklndl"//"jpcvzwgj"//
#define mqtt_password 		"x9aBwks70kmQ"//"JCCSw9dYngMF"//"2k756Wus2bJE"///


#define TP_PROVISION_START          "RD/PROVISION/START"    // {"PRO":"1","VALUE":"1"}
#define TP_PROVISION_STOP           "RD/PROVISION/STOP"     // {"PRO":"1","VALUE":"0"}
#define TP_CONTROL_RESETNODE		"RD/CONTROL/RESETNODE"  // {"ADR":"adr","VALUE":"1"}
#define TP_CONTROL_UPDATE		"RD/CONTROL/UPDATE"         // {"ADR":"65535","VALUE":"1"}
#define TP_CONTROL_ONOFF        "RD/CONTROL/ONOFF"          // {"ADR":"adr","VALUE":"1 0r 0"}
#define TP_CONTROL_CCT        	"RD/CONTROL/CCT"			// {"ADR":"adr","VALUE":"cct"}
#define TP_CONTROL_DIM   		"RD/CONTROL/DIM"			// {"ADR":"adr","VALUE":"dim"}
#define TP_CONTROL_HSL   		"RD/CONTROL/HSL"
#define TP_CONTROL_ADDGROUP   	"RD/CONTROL/ADDGROUP"		// {"ADR":"adr","VALUE":"address group"}
#define TP_CONTROL_DELGROUP   	"RD/CONTROL/DELGROUP"		// {"ADR":"adr","VALUE":"address group"}
#define TP_CONTROL_ADDSENCE   	"RD/CONTROL/ADDSENCE"       // {"ADR":"adr","VALUE":"id sence"}
#define TP_CONTROL_CALLSENCE    "RD/CONTROL/CALLSENCE"      // {"IDSENCE":"idsence","VALUE":"1"}
#define TP_CONTROL_DELSENCE   	"RD/CONTROL/DELSENCE"		// {"ADR":"adr","VALUE":"id sence"}
#define TP_CONTROL_RESETNODE   	"RD/CONTROL/RESETNODE"		// {"ADR":"adr","VALUE":"1"}
#define TP_CONTROL_SCHEDULE     "RD/CONTROL/SHEDULE"		// {"ADR":"adr","HOURS":"hours","MINUTES":"minutes","SECOND":"second","ONOFF":"x","DIM":"xx,"CCT":"xx"}

#define TP_TPYE_DEVICE      "RD/TYPE_DEVICE"            // {"ADR":"adr","MMAINDEVICE":"main","SUBDEVICE":"sub","POWER":"power",}
#define TP_STATUS_UPDATE    "RD/STATUS/UPDATE"			//{"ADR":"adr","ONOFF":"1 or 0"} gửi nhiều lần của từng con đèn trong mạng
#define TP_STATUS_ONOFF   	"RD/STATUS/ONOFF"           //{"ADR":"adr","ONOFF":"1 or 0"}
#define TP_STATUS_CCT   	"RD/STATUS/CCT"				//{"ADR":"adr","CCT":"cct"}
#define TP_STATUS_DIM   	"RD/STATUS/DIM"				//{"ADR":"adr","DIM":"dim"}
#define TP_STATUS_HSL   	"RD/STATUS/HSL"
#define TP_STATUS_ADDGROUP   "RD/STATUS/ADDGROUP"       //{"ADR":"adr","ADDGROUP":"address group"}
#define TP_STATUS_DELGROUP   "RD/STATUS/DELGROUP"		//{"ADR":"adr","DELGROUP":"address group"}
#define TP_STATUS_ADDSENCE   "RD/STATUS/ADDSENCE"       //{"ADR":"adr","ADDSENCE":"id sence"}
#define TP_STATUS_CALLSENCE	 "RD/STATUS/CALLSENCE"      //{"ADR":"adr","CALLSENCE":"id sence"}
#define TP_STATUS_DELSENCE   "RD/STATUS/DELSENCE"
#define TP_STATUS_RESETNODE  "RD/STATUS/RESETNODE"      //{"ADR":"adr","RESETNODE":"1"}
#define TP_STATUS_SHEDULE    "RD/STATUS/SHEDULE"		//{"ADR":"adr","HOURS":"hours","MINUTES":"minutes","SECOND":"second","ONOFF":"x","DIM":"xx,"CCT":"xx"}

extern int run;

extern bool flag_on_all;
extern bool flag_off_all;
extern struct mosquitto *mosq;

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
