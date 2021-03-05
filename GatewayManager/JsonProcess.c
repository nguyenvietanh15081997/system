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
uint16_t numObject;
jsonstring vrts_Json_String;
defineCmd flagDefineCmd;
pthread_t vrts_System_TestSend;

char flagSecond=0;
/*
 * TODO: xử lý chuỗi json phức tạp không dùng delay
 */
void JsonControl(char *key){
	if(strcmp(key,"ONOFF")==0){
		 flagDefineCmd = onoff_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,vrts_Json_String.adr ,NULL8, vrts_Json_String.onoff, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 if(strcmp(key,"CCT")==0){
		 flagDefineCmd = cct_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16,vrts_Json_String.cct, NULL16, NULL16,NULL16, NULL16, NULL16, 17);
		 sleep(1);
	 }
	 if(strcmp(key,"DIM")==0){
		 flagDefineCmd = dim_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, Lightness_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, vrts_Json_String.dim, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 14);
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
	 if(strcmp(key,"ADDGROUP")==0){
		 flagDefineCmd = addgroup_enum;
		 check_add_or_del_group=true;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, AddGroup_typedef, vrts_Json_String.adr, vrts_Json_String.addgroup , NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
		 sleep(1);
	 }
	 if(strcmp(key,"DELGROUP")==0){
		 flagDefineCmd = delgroup_enum;
		 check_add_or_del_group=false;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, DelGroup_typedef, vrts_Json_String.adr, vrts_Json_String.delgroup, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 18);
		 sleep(1);
	 }
	 if(strcmp(key,"ADDSCENE")==0){
		 flagDefineCmd = addscene_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.addscene, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 if(strcmp(key,"CALLSCENE")==0){
		 flagDefineCmd = callscene_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.callscene, NULL16,NULL16, NULL16, NULL16, 17);
		 sleep(1);
	 }
	 if(strcmp(key,"DELSCENE")==0){
		 flagDefineCmd = delscene_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.delscene, NULL16,NULL16, NULL16, NULL16, 14);
		 sleep(1);
	 }
	 if(strcmp(key,"HUE")==0){
		 flagDefineCmd = hue_eum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, HSL_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,vrts_Json_String.lightness, vrts_Json_String.hue, vrts_Json_String.saturation, 19 );
	 }
	 if(strcmp(key,"RESET")==0){
		 flagDefineCmd = resetnode_enum;
		 FunctionPer(HCI_CMD_GATEWAY_CMD, ResetNode_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, 12);
	 }
	 if(strcmp(key,"START")==0){
		flagDefineCmd = start_enum;
		slog_print(SLOG_NOTAG, 1, "<provision>Provision start");
		MODE_PROVISION=true;
		pthread_create(&vrts_System_TestSend,NULL, ProvisionThread, NULL);
		//pthread_join(vrts_System_TestSend, NULL);
	 }
	 if(strcmp(key,"STOP")==0){
		 flagDefineCmd = stop_enum;
		 slog_print(SLOG_NOTAG, 1, "<provision>Provision stop");
		MODE_PROVISION=false;
		pthread_cancel(tmp);
		flag_blink=false;
		pthread_cancel(tmp1);
		ControlMessage(3, OUTMESSAGE_ScanStop);
		flag_selectmac     = false;
		flag_getpro_info   = false;
		flag_getpro_element= false;
		flag_provision     = false;
		flag_mac           = true;
		flag_check_select_mac  = false;
		flag_done          = true;
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
				 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,28);
	 }
	 if((strcmp(key,"TYPEDEVICESCAN")==0)){
		 Function_Vendor(HCI_CMD_GATEWAY_CMD, AskTypeDevice_vendor_typedef, NULL16, NULL16,\
				 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,28);
	 }
	 if((strcmp(key,"SETTYPEDEVICE")==0)){
		 Function_Vendor(HCI_CMD_GATEWAY_CMD, SetTypeDevice_vendor_typedef, vrts_Json_String.adr, NULL16, \
				 NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8,\
				 vrts_Json_String.type, vrts_Json_String.attrubute, vrts_Json_String.application,28);
	 }
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
								 vrts_Json_String.start 		= (json_object_get_int(json_object_object_get(jobj,"START")));
								 vrts_Json_String.stop 			= (json_object_get_int(json_object_object_get(jobj,"STOP")));
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
								 break;
			 }
			 JsonControl(key);
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
