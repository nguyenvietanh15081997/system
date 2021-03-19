/*
 * Json.c
 */

#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/MQTT.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/slog.h"
#include "../GatewayManager/LedProcess.h"


uint16_t valueObject[20];
uint16_t numObject,scene_id;
jsonstring vrts_Json_String;
defineCmd flagDefineCmd;
pthread_t vrts_System_TestSend;

bool arr_json = false;
bool flag_addscene = false;
bool flag_delscene = false;

bool check_resetnode = false;
uint16_t adr_dv[100];
int i, arraylen, lst_count_id;

char flagSecond=0;


/*
 * TODO: xử lý chuỗi json phức tạp không dùng delay
 */
void JsonControl(json_object *jobj,char *key){
	if(strcmp(key,"ADR")==0){
		if(check_resetnode)
		{
			check_resetnode = false;
			 FunctionPer(HCI_CMD_GATEWAY_CMD, ResetNode_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
		}
	}
	if(strcmp(key,"ONOFF")==0){
		 flagDefineCmd = onoff_enum;
		 //FunctionPer(HCI_CMD_GATEWAY_CMD,CCT_Get_typedef,vrts_Json_String.adr ,NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
		 FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,vrts_Json_String.adr ,NULL8, vrts_Json_String.onoff, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 if(strcmp(key,"CCT")==0){
		 flagDefineCmd = cct_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16,Percent2ParamCCT(vrts_Json_String.cct), NULL16, NULL16,NULL16, NULL16, NULL16, 17);
		 sleep(1);
	 }
	 if(strcmp(key,"DIM")==0){
		 flagDefineCmd = dim_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, Lightness_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, Percent2ParamDIM(vrts_Json_String.dim), NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 if(strcmp(key,"HOURS")==0){
		 flagDefineCmd = hours_enum;
	 }
	 if(strcmp(key,"MINUTES")==0){
		 flagDefineCmd = minutes_enum;
	 }
	 if(strcmp(key,"SECONDS")==0){
		 flagDefineCmd = seconds_enum;
		 flagSecond= 1;
	 }
//	 if(strcmp(key,"ADDGROUP")==0){
//		 flagDefineCmd = addgroup_enum;
//		 check_add_or_del_group=true;
//		 FunctionPer(HCI_CMD_GATEWAY_CMD, AddGroup_typedef, vrts_Json_String.adr, vrts_Json_String.addgroup , NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
//		 sleep(1);
//	 }
	 if(strcmp(key,"ADDGROUP")==0){
		 flagDefineCmd = addgroup_enum;
		 check_add_or_del_group= true;
		 for(i=0; i<arraylen; i++)
		 {
			 FunctionPer(HCI_CMD_GATEWAY_CMD, AddGroup_typedef, adr_dv[i], vrts_Json_String.addgroup , NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
			 //printf("adr_dv: %d\n",adr_dv[i]);
			 usleep(500000);
		 }
	 }
//	 if(strcmp(key,"DELGROUP")==0){
//		 flagDefineCmd = delgroup_enum;
//		 check_add_or_del_group=false;
//		 FunctionPer(HCI_CMD_GATEWAY_CMD, DelGroup_typedef, vrts_Json_String.adr, vrts_Json_String.delgroup, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
//		 sleep(1);
//	 }
	 if(strcmp(key,"DELGROUP")==0){
		 flagDefineCmd = delgroup_enum;
		 check_add_or_del_group= false;
		 for(i=0; i<arraylen; i++)
		 {
			 FunctionPer(HCI_CMD_GATEWAY_CMD, DelGroup_typedef, adr_dv[i], vrts_Json_String.delgroup, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
			 usleep(500000);
		 }
	 }
//	 if(strcmp(key,"ADDSCENE")==0){
//		 flagDefineCmd = addscene_enum;
//		 FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.addscene, NULL16,NULL16, NULL16, NULL16, 14);
//		 sleep(1);
//	 }
	 if(strcmp(key,"ADDSCENE")==0){
		 flagDefineCmd = addscene_enum;
		 scene_id = vrts_Json_String.addscene;
		 flag_addscene = true;
		 check_add_or_del_scene= true;
	 	 }
	 if(strcmp(key,"CALLSCENE")==0){
		 flagDefineCmd = callscene_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.callscene, NULL16,NULL16, NULL16, NULL16, 17);
		 sleep(1);
	 }
//	 if(strcmp(key,"DELSCENE")==0){
//		 flagDefineCmd = delscene_enum;
//		 FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.delscene, NULL16,NULL16, NULL16, NULL16, 14);
//		 sleep(1);
//	 }
	 if(strcmp(key,"DELSCENE")==0){
		 flagDefineCmd = delscene_enum;
		 check_add_or_del_scene= false;
		 for(i=0; i<arraylen; i++)
		 {
			 FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, adr_dv[i], NULL8, NULL8, NULL16, NULL16, vrts_Json_String.delscene, NULL16,NULL16, NULL16, NULL16, 14);
			 usleep(500000);
		 }
	 }
	 if(strcmp(key,"HUE")==0){
		 flagDefineCmd = hue_eum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, HSL_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,vrts_Json_String.lightness, vrts_Json_String.hue, vrts_Json_String.saturation, 19 );
	 }
	 if(strcmp(key,"CMD")==0){
		 if(strcmp(vrts_Json_String.cmd,"SCAN")==0){
			slog_print(SLOG_INFO, 1, "<provision>Provision start");
			MODE_PROVISION=true;
			pthread_create(&vrts_System_TestSend,NULL, ProvisionThread, NULL);
			//pthread_join(vrts_System_TestSend, NULL);
		 }
		 if(strcmp(vrts_Json_String.cmd,"STOP")==0){
			slog_print(SLOG_INFO, 1, "<provision>Provision stop");
			MODE_PROVISION=false;
			ControlMessage(3, OUTMESSAGE_ScanStop);
			puts("1");
			//pthread_cancel(tmp);
			puts("2");
			flag_selectmac     = false;
			flag_getpro_info   = false;
			flag_getpro_element= false;
			flag_provision     = false;
			flag_mac           = true;
			flag_check_select_mac  = false;
			flag_done          = true;
			flag_set_type = false;
			//for gpio
			flag_blink = false;
			puts("3");
			//pthread_create(&tmp1,NULL,Led_Thread,NULL);
			puts("4");
			led_pin_off(gpio[LED_BLE_PIN_INDEX]);
			puts("5");
		 }
		 if(strcmp(vrts_Json_String.cmd,"RESETNODE")==0){
			 check_resetnode = true;
		 }
	 }
	 if(strcmp(key,"UPDATE")==0){
		 flagDefineCmd = update_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, UpdateLight_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
	 }
	 if((strcmp(key,"SCENEFORREMOTE")==0)){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRemote_vendor_typedef, vrts_Json_String.adr, NULL16, vrts_Json_String.buttonid,\
				vrts_Json_String.modeid, NULL8, NULL16, NULL16, NULL16,\
				NULL16, vrts_Json_String.sceneID, vrts_Json_String.appID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8,31);
	 }
	 if((strcmp(key,"SCENEFORSENSOR")==0)){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForSensor_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8,\
				NULL8, vrts_Json_String.stt, vrts_Json_String.condition, vrts_Json_String.low_lux, vrts_Json_String.hight_lux,\
				vrts_Json_String.action, vrts_Json_String.sceneID, vrts_Json_String.appID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8,31);
	 }
	 if((strcmp(key,"SETSCENERGB")==0)){
		 Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRGB_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, \
				 NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.appID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, 23);
	 }
	 if((strcmp(key,"CALLSCENERGB")==0)){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8,NULL8, NULL8, NULL16,\
				NULL16, NULL16,NULL16, NULL16, vrts_Json_String.callsceneRGB, NULL8, NULL8, NULL8, NULL8,23);
	 }
	 if((strcmp(key,"CALLMODERGB")==0)){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, CallModeRgb_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8,NULL8, NULL8, NULL16,\
				NULL16, NULL16,NULL16, NULL16, NULL16, vrts_Json_String.callmodeRGB, NULL8, NULL8, NULL8,23);
	 }
	 if((strcmp(key,"DELSCENERGB")==0)){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneRgb_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8,NULL8, NULL8, NULL16,\
				NULL16, NULL16,NULL16, NULL16, vrts_Json_String.delsceneRGB, NULL8, NULL8, NULL8, NULL8,23);
	 }
	 if((strcmp(key,"SAVEGATEWAY")==0)){
		 Function_Vendor(HCI_CMD_GATEWAY_CMD, SaveGateway_vendor_typedef, vrts_Json_String.adr, NULL16,\
				 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,17);
	 }
	 if((strcmp(key,"TYPEDEVICESCAN")==0)){
		 Function_Vendor(HCI_CMD_GATEWAY_CMD, AskTypeDevice_vendor_typedef, vrts_Json_String.adr, NULL16,\
				 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,17);
	 }
	 if((strcmp(key,"SETTYPEDEVICE")==0)){
		 Function_Vendor(HCI_CMD_GATEWAY_CMD, SetTypeDevice_vendor_typedef, vrts_Json_String.adr, NULL16, \
				 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8,\
				 vrts_Json_String.type, vrts_Json_String.attrubute, vrts_Json_String.application,28);
	 }
}
void json_value(json_object *jobj)
{
	enum json_type type;
	type = json_object_get_type(jobj);
	switch (type)
	{
		case json_type_int:
			adr_dv[i] = json_object_get_int(jobj);
			break;
	}
}

int json_parse_array( json_object *jobj, char *key)
{
	enum json_type type;

	json_object *jarray = jobj;
	if(key)
	{
		jarray = json_object_object_get(jobj, key);
	}

	arraylen = json_object_array_length(jarray);
	//printf("Array Length: %d\n",arraylen);
	json_object * jvalue;

	for (i=0; i< arraylen; i++)
	{
		jvalue = json_object_array_get_idx(jarray, i);
		type = json_object_get_type(jvalue);
		if (type != json_type_object)
		{
			json_value(jvalue);
		}
		else if(type == json_type_object)
		{
			vrts_Json_String.id        		= (json_object_get_int(json_object_object_get(jvalue,"id")));
			vrts_Json_String.cct 			= (json_object_get_int(json_object_object_get(jvalue,"CCT")));
			vrts_Json_String.dim 			= (json_object_get_int(json_object_object_get(jvalue,"DIM")));
			FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_typedef, vrts_Json_String.id+1, NULL8, NULL8, NULL16,Percent2ParamCCT(vrts_Json_String.cct), NULL16, NULL16,NULL16, NULL16, NULL16, 17);sleep(1);
			FunctionPer(HCI_CMD_GATEWAY_CMD, Lightness_Set_typedef, vrts_Json_String.id, NULL8, NULL8, Percent2ParamDIM(vrts_Json_String.dim), NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);sleep(1);
			FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef, vrts_Json_String.id, NULL8, NULL8, NULL16, NULL16, scene_id, NULL16,NULL16, NULL16, NULL16, 14);sleep(1);
		}
	}
	return 1;
}

/*
 * TODO: cần xử lý kiểu object(object)
 */
void Json_Parse(json_object * jobj)
{
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
								 vrts_Json_String.resetnode 	= (json_object_get_int(json_object_object_get(jobj,"RESET")));
//								 vrts_Json_String.start 		= (json_object_get_int(json_object_object_get(jobj,"START")));
//								 vrts_Json_String.stop 			= (json_object_get_int(json_object_object_get(jobj,"STOP")));
								 vrts_Json_String.update 		= (json_object_get_int(json_object_object_get(jobj,"UPDATE")));
								 vrts_Json_String.header        = (json_object_get_int(json_object_object_get(jobj,"HEADER")));
								 vrts_Json_String.stt           = (json_object_get_int(json_object_object_get(jobj,"STT")));
								 vrts_Json_String.action        = (json_object_get_int(json_object_object_get(jobj,"ACTION")));
								 vrts_Json_String.condition     = (json_object_get_int(json_object_object_get(jobj,"CONDITION")));
								 vrts_Json_String.low_lux       = (json_object_get_int(json_object_object_get(jobj,"LOW_LUX")));
								 vrts_Json_String.hight_lux     = (json_object_get_int(json_object_object_get(jobj,"HIGHT_LUX")));
								 vrts_Json_String.buttonid      = (json_object_get_int(json_object_object_get(jobj,"BUTTONID")));
								 vrts_Json_String.modeid        = (json_object_get_int(json_object_object_get(jobj,"MODEID")));
								 vrts_Json_String.srgbID        = (json_object_get_int(json_object_object_get(jobj,"SRGBID")));
								 vrts_Json_String.appID         = (json_object_get_int(json_object_object_get(jobj,"APPID")));
								 vrts_Json_String.sceneID       = (json_object_get_int(json_object_object_get(jobj,"SCENEID")));

								 vrts_Json_String.sceneforremote= (json_object_get_int(json_object_object_get(jobj,"SCENEFORREMOTE")));
								 vrts_Json_String.sceneforsensor= (json_object_get_int(json_object_object_get(jobj,"SCENEFORSENSOR")));

								 vrts_Json_String.setsceneRGB   = (json_object_get_int(json_object_object_get(jobj,"SETSCENERGB")));
								 vrts_Json_String.callsceneRGB  = (json_object_get_int(json_object_object_get(jobj,"CALLSCENERGB")));
								 vrts_Json_String.callmodeRGB  = (json_object_get_int(json_object_object_get(jobj,"CALLMODERGB")));
								 vrts_Json_String.delsceneRGB  = (json_object_get_int(json_object_object_get(jobj,"DELSCENERGB")));

								 vrts_Json_String.savegateway   = (json_object_get_int(json_object_object_get(jobj,"SAVEGATEWAY")));
								 vrts_Json_String.settypedevice = (json_object_get_int(json_object_object_get(jobj,"SETTYPEDEVICE")));
								 vrts_Json_String.typedevicescan = (json_object_get_int(json_object_object_get(jobj,"TYPEDEVICESCAN")));
								 vrts_Json_String.type      = (json_object_get_int(json_object_object_get(jobj,"TYPE")));
								 vrts_Json_String.attrubute       = (json_object_get_int(json_object_object_get(jobj,"ATTRUBUTE")));
								 vrts_Json_String.application   = (json_object_get_int(json_object_object_get(jobj,"APPLICATION")));
								 JsonControl(jobj,key);
								 break;
							 case json_type_string:
								 vrts_Json_String.cmd 	= (json_object_get_string(json_object_object_get(jobj,"CMD")));
								 JsonControl(jobj, key);
								 break;
							 case json_type_array:
									//printf("type: json_type_array\n");
									json_parse_array(jobj, key);
									break;
			 }
			 //JsonControl(key);
		 }
}
/*
 * TODO:cần phản hồi đa dạng hơn
 * - hiện tại mới tách nhiều bản tin
 * - mỗi bản tin chỉ có {"KEY":value}
 * - chờ quyết định dạng bản tin truyền nhận bên trên
 */
void CreatJson(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,uint16_t par1, uint16_t par2)
{
	struct json_object * object;
	object = json_object_new_object();
	json_object_object_add(object, objectJsonAdr, json_object_new_int(par1));
	json_object_object_add(object, objectJsonValue, json_object_new_int(par2));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
	slog_info("(mqtt)Message_send:%s",rsp);
}
void CreatJsonString(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,char * par1, uint16_t par2)
{
	struct json_object * object;
	object = json_object_new_object();
	json_object_object_add(object, objectJsonAdr, json_object_new_string(par1));
	json_object_object_add(object, objectJsonValue, json_object_new_int(par2));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
	slog_info("(mqtt)Message_send:%s",rsp);
}
void CreatJsonString_2(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue , uint16_t par1, char * par2)
{
	struct json_object * object;
	object = json_object_new_object();
	json_object_object_add(object, objectJsonAdr, json_object_new_int(par1));
	json_object_object_add(object, objectJsonValue, json_object_new_string(par2));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
	slog_info("(mqtt)Message_send:%s",rsp);
}
/*
 * TODO:chỉ cho lúc phản hồi bản tin type device
 */
void CreatJson_TypeDev(uint8_t *topic, uint8_t *objectJsonAdr, uint8_t *objectJsonType, uint8_t *objectJsonAttrubute,\
		uint8_t *objectJsonApplication, uint16_t parAdr, uint16_t parType, uint16_t parAttrubute, uint16_t parApplication)
{
	struct json_object * object;
	object = json_object_new_object();
	json_object_object_add(object, objectJsonAdr, json_object_new_int(parAdr));
	json_object_object_add(object, objectJsonType, json_object_new_int(parType));
	json_object_object_add(object, objectJsonAttrubute, json_object_new_int(parAttrubute));
	json_object_object_add(object, objectJsonApplication, json_object_new_int(parApplication));
	char *rsp;
	rsp = json_object_to_json_string(object);
	mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
	slog_info("(mqtt)Message_send:%s",rsp);
}
void CreatJson_New_TypeDev(uint8_t *topic,uint8_t * key1, uint8_t * key2, uint8_t * key3, uint8_t * key4,\
		uint8_t * key5,uint8_t * key6,uint8_t * key7,uint8_t * key8,uint16_t value1, uint8_t *value2,uint8_t *value3,\
		 uint8_t *value4, uint8_t *value5,uint16_t value6,uint8_t *value7){
	struct json_object * object;
		struct json_object * object1;
		object = json_object_new_object();
		object1 = json_object_new_object();
		json_object_object_add(object1, key1, json_object_new_int(value1));
		json_object_object_add(object1, key2, json_object_new_string(value2));
		json_object_object_add(object1, key3, json_object_new_string(value3));
		json_object_object_add(object1, key4, json_object_new_string(value4));
		json_object_object_add(object1, key5, json_object_new_string(value5));
		json_object_object_add(object1, key6, json_object_new_int(value6));
		json_object_object_add(object, key7, json_object_new_string(value7));
		json_object_object_add(object, key8,object1);
		char *rsp;
		rsp = json_object_to_json_string(object);
		mosquitto_publish(mosq, NULL, topic, strlen(rsp), rsp, 0, 0);
		slog_info("(mqtt)Message_send:%s",rsp);
}
