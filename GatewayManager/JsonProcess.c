#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/MQTT.h"


uint16_t valueJsonRec[2];


void Json_Parse(json_object * jobj)
{
	puts("ok");
	int i = 0;
	 enum json_type type;
	 json_object_object_foreach(jobj, key, val)
	 {
		 type = json_object_get_type(val);
		 switch (type)
		 {
			 case json_type_string:
				valueJsonRec[i]=atoi(json_object_get_string(val));
				i++;
				break;
		 }
	 }
	 printf("Data:");
	 int j;
	 for(j=0; j<2;j++)
	 {
		 printf("%d ", valueJsonRec[j]);
	 }
	 puts("done");
}
void CreatJson(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,uint16_t par1, uint16_t par2)
{
	struct json_object * object;
	char valueJsonSend[2];
	sprintf(valueJsonSend,"%d",par1);
	object = json_object_new_object();
	json_object_object_add(object, objectJsonAdr, json_object_new_string(valueJsonSend));
	sprintf(valueJsonSend,"%d",par2);
	json_object_object_add(object, objectJsonValue, json_object_new_string(valueJsonSend));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
}
void CreatJson_TypeDev(uint8_t *topic, uint8_t *objectJsonAdr, uint8_t *objectJsonMain, uint8_t *objectJsonSub, uint8_t *objectJsonPower, uint8_t parAdr, uint8_t parMain, uint8_t parSub, uint8_t parPower)
{
	struct json_object * object;
	object = json_object_new_object();
	char valueJsonSend[4];
	sprintf(valueJsonSend,"%d",parAdr);
	json_object_object_add(object, objectJsonAdr, json_object_new_string(valueJsonSend));
	sprintf(valueJsonSend,"%d",parMain);
	json_object_object_add(object, objectJsonMain, json_object_new_string(valueJsonSend));
	sprintf(valueJsonSend,"%d",parSub);
	json_object_object_add(object, objectJsonSub, json_object_new_string(valueJsonSend));
	sprintf(valueJsonSend,"%d",parPower);
	json_object_object_add(object, objectJsonPower, json_object_new_string(valueJsonSend));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
}
