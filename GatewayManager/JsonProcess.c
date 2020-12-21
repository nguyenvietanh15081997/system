#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/MQTT.h"
#include "../GatewayManager/Light.h"


uint16_t valueObject[20];
uint16_t numObject;
jsonstring vrts_Json_String;
defineCmd flagDefineCmd;

void JsonControl(char *key){
	if(strcmp(key,"ONOFF")==0){
		 flagDefineCmd = onoff_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,vrts_Json_String.adr ,NULL8, vrts_Json_String.onoff, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 else if(strcmp(key,"CCT")==0){
		 flagDefineCmd = cct_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16,vrts_Json_String.cct, NULL16, NULL16,NULL16, NULL16, NULL16, 17);
		 sleep(1);
	 }
	 else if(strcmp(key,"DIM")==0){
		 flagDefineCmd = dim_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, Lightness_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, vrts_Json_String.dim, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 else if(strcmp(key,"HOURS")==0){
		 puts("4");
		 flagDefineCmd = hours_enum;
	 }
	 else if(strcmp(key,"MINUTES")==0){
		 flagDefineCmd = minutes_enum;
	 }
	 else if(strcmp(key,"SECONDS")==0){
		 flagDefineCmd = seconds_enum;
	 }
	 else if(strcmp(key,"ADDGROUP")==0){
		 flagDefineCmd = addgroup_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, AddGroup_typedef, vrts_Json_String.adr, vrts_Json_String.addgroup , NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
		 sleep(1);
	 }
	 else if(strcmp(key,"DELGROUP")==0){
		 flagDefineCmd = delgroup_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, DelGroup_typedef, vrts_Json_String.adr, vrts_Json_String.delgroup, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
		 sleep(1);
	 }
	 else if(strcmp(key,"ADDSCENE")==0){
		 flagDefineCmd = addscene_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.addscene, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 else if(strcmp(key,"CALLSCENE")==0){
		 flagDefineCmd = callscene_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.callscene, NULL16,NULL16, NULL16, NULL16, 17);
		 sleep(1);
	 }
	 else if(strcmp(key,"DELSCENE")==0){
		 flagDefineCmd = delscene_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.delscene, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 else if(strcmp(key,"HUE")==0){
		 flagDefineCmd = hue_eum;
	 }
	 else if(strcmp(key,"SATURATION")==0){
		 flagDefineCmd = saturation_enum;
	 }
	 else if(strcmp(key,"LIGHTNESS")==0){
		 flagDefineCmd = lightness_enum;
	 }
	 else if(strcmp(key,"RESETNODE")==0){
		 flagDefineCmd = resetnode_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, ResetNode_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
	 }
	 else if(strcmp(key,"START")==0){
		 flagDefineCmd = start_enum;
	 }
	 else if(strcmp(key,"STOP")==0){
		 flagDefineCmd = stop_enum;
	 }
	 else if(strcmp(key,"UPDATE")==0){
		 flagDefineCmd = update_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, UpdateLight_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
	 }
}
void Json_Parse(json_object * jobj)
{
	puts("ok");
	enum json_type type;
		 json_object_object_foreach(jobj, key, val) {
			 type= json_object_get_type(val);
			 switch(type){
							 case json_type_int:
								 vrts_Json_String.adr        	= (json_object_get_int(json_object_object_get(jobj,"ADR")));
								 vrts_Json_String.onoff 		= (json_object_get_int(json_object_object_get(jobj,"ONOFF")));
								 vrts_Json_String.cct 			= (json_object_get_int(json_object_object_get(jobj,"CCT")));
								 vrts_Json_String.dim 			= (json_object_get_int(json_object_object_get(jobj,"DIM")));
								 vrts_Json_String.hours 		= (json_object_get_int(json_object_object_get(jobj,"HOURS")));
								 vrts_Json_String.minutes 		= (json_object_get_int(json_object_object_get(jobj,"MINUTES")));
								 vrts_Json_String.seconds		= (json_object_get_int(json_object_object_get(jobj,"SECONDS")));
								 vrts_Json_String.addgroup 		= (json_object_get_int(json_object_object_get(jobj,"ADDGROUP")));
								 vrts_Json_String.delgroup 		= (json_object_get_int(json_object_object_get(jobj,"DELGROUP")));
								 vrts_Json_String.addscene 		= (json_object_get_int(json_object_object_get(jobj,"ADDSCENE")));
								 vrts_Json_String.callscene 	= (json_object_get_int(json_object_object_get(jobj,"CALLSCENE")));
								 vrts_Json_String.delscene 		= (json_object_get_int(json_object_object_get(jobj,"DELSCENE")));
								 vrts_Json_String.hue 			= (json_object_get_int(json_object_object_get(jobj,"HUE")));
								 vrts_Json_String.saturation	= (json_object_get_int(json_object_object_get(jobj,"SATURATION")));
								 vrts_Json_String.lightness 	= (json_object_get_int(json_object_object_get(jobj,"LIGHTNESS")));
								 vrts_Json_String.resetnode 	= (json_object_get_int(json_object_object_get(jobj,"RESETNODE")));
								 vrts_Json_String.start 		= (json_object_get_int(json_object_object_get(jobj,"START")));
								 vrts_Json_String.stop 			= (json_object_get_int(json_object_object_get(jobj,"STOP")));
								 vrts_Json_String.update 		= (json_object_get_int(json_object_object_get(jobj,"UPDATE")));
								 break;
			 }
			 JsonControl(key);
		 }
		 //printf("Data:%d %d %d %d\n",vrts_Json_String.adr,vrts_Json_String.cct,vrts_Json_String.dim,vrts_Json_String.start);
		 puts("done");
}
void CreatJson(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,uint16_t par1, uint16_t par2)
{
	struct json_object * object;
	//char valueJsonSend[2];
	//sprintf(valueJsonSend,"%d",par1);
	object = json_object_new_object();
	json_object_object_add(object, objectJsonAdr, json_object_new_int(par1));
	//sprintf(valueJsonSend,"%d",par2);
	json_object_object_add(object, objectJsonValue, json_object_new_int(par2));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
}
void CreatJson_TypeDev(uint8_t *topic, uint8_t *objectJsonAdr, uint8_t *objectJsonMain, uint8_t *objectJsonSub, uint8_t *objectJsonPower, uint8_t parAdr, uint8_t parMain, uint8_t parSub, uint8_t parPower)
{
	struct json_object * object;
	object = json_object_new_object();
	//char valueJsonSend[4];
	//sprintf(valueJsonSend,"%d",parAdr);
	json_object_object_add(object, objectJsonAdr, json_object_new_int(parAdr));
	//sprintf(valueJsonSend,"%d",parMain);
	json_object_object_add(object, objectJsonMain, json_object_new_int(parMain));
	//sprintf(valueJsonSend,"%d",parSub);
	json_object_object_add(object, objectJsonSub, json_object_new_int(parSub));
	//sprintf(valueJsonSend,"%d",parPower);
	json_object_object_add(object, objectJsonPower, json_object_new_int(parPower));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
}
