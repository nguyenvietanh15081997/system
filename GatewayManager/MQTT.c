/*
 * MQTT.c
 */

#include "../GatewayManager/MQTT.h"
#include "../GatewayManager/OpCode.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/slog.h"

char *pHeaderMqtt = "mqtt";
pthread_t vrts_System_TestSend;
struct mosquitto *mosq;
unsigned char qos =2;
int run = 1;
void handle_signal(int s)
{
	run = 0;
}
int mqtt_send(struct mosquitto *mosq, char * topic,char *msg)
{
	mosquitto_publish(mosq, NULL,topic, strlen(msg), msg, qos, 0);
	slog_info("(%s)Message_send: %s",pHeaderMqtt,msg);
	return 0;
}
void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	slog_info("(%s)%s: Connect callback, rc=%d",pHeaderMqtt,mqtt_host,result);
	mqtt_send(mosq,"RD_STATUS","{\"CMD\":\"CONNECTED\"}");
}
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	char* msg = (char*)message->payload;
	slog_info("(%s)Message_receive: %s",pHeaderMqtt,msg);
	if(json_tokener_parse(msg)!= NULL)
	{
		struct json_obj* jobj = json_tokener_parse(msg);
		Json_Parse(jobj);
	}
}

void * MQTT_Thread(void *argv)
{
		char clientid[24];
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

			mosquitto_subscribe(mosq,NULL, "RD_CONTROL",qos);
			while(run){
				rc= abc = mosquitto_loop(mosq, -1, 1);
				if(run && rc){
					slog_warn("Connection mqtt error");
					sleep(4);
					mosquitto_reconnect(mosq);
				}
			}
			mosquitto_destroy(mosq);
		}
		mosquitto_lib_cleanup();
		return NULL;
}

