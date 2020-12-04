#include "../GatewayManager/JsonProcess.h"


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
void CreatJson(struct json_object * topic,uint8_t * objectJson,uint8_t par1, uint8_t par2)
{
	char valueJsonSend[2];
	sprintf(valueJsonSend,"%d%d",par1,par2);
	topic = json_object_new_object();
	json_object_object_add(topic, objectJson, json_object_new_string(valueJsonSend));
	char *rsp;
	rsp = json_object_to_json_string(topic);
}
