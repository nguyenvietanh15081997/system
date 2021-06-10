/*
 * Json.c
 */

#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/MQTT.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/Provision.h"
#include "../GatewayManager/slog.h"
#include "../GatewayManager/LedProcess.h"

jsonstring vrts_Json_String;
pthread_t vrts_System_Provision;

bool flag_addscene = false;
bool flag_delscene = false;
static bool flag_check_device_unicast_id = false;
static bool flag_check_device_rgb_unicast_id = false;
static bool flag_scene_cct = false;
static bool flag_scene_rgbcct = false;

static uint16_t adr_dv[100];
static uint16_t adr_rgb_cct_dv[100];
static uint16_t adr_rgb_dv[100];
int i, arraylen, arraylen_rgb, arraylen_rgb_cct;

int json_parse_array( json_object *jobj, char *key)
{
	enum json_type type;
	if(strcmp(key,"DEVICE_UNICAST_ID") == 0){
		flag_check_device_unicast_id = true;
		json_object *jarray = json_object_object_get(jobj, key);
		arraylen = json_object_array_length(jarray);
		for (i=0; i< arraylen; i++){
			json_object *jvalue = json_object_array_get_idx(jarray, i);
			type = json_object_get_type(jvalue);
			if (type == json_type_int){
				adr_dv[i] = json_object_get_int(jvalue);
				//printf("adr[%d]= %d\n",i,adr_dv[i]);
			}
		}
	}
	if(strcmp(key,"DEVICE_RGB_UNICAST_ID") == 0){
		flag_check_device_rgb_unicast_id = true;
		json_object *jarray_rgb = json_object_object_get(jobj, key);
		arraylen_rgb = json_object_array_length(jarray_rgb);
		for (i=0; i< arraylen_rgb; i++){
			json_object *jvalue_rgb = json_object_array_get_idx(jarray_rgb, i);
			type = json_object_get_type(jvalue_rgb);
			if (type == json_type_int){
				adr_rgb_dv[i] = json_object_get_int(jvalue_rgb);
				//printf("adr_rgb[%d]= %d\n",i,adr_rgb_dv[i]);
			}
		}
	}
	if(strcmp(key, "RGB_CCT") ==0 ){
		flag_scene_rgbcct = true;
		//puts("check array rgb_cct");
//		json_object *jarray_rgb = json_object_object_get(jobj, key);
//		arraylen_rgb = json_object_array_length(jarray_rgb);
//		printf("number: %d\n",arraylen_rgb);
//		for (i=0; i< arraylen_rgb; i++){
//			json_object *jvalue_rgb = json_object_array_get_idx(jarray_rgb, i);
//			type = json_object_get_type(jvalue_rgb);
//			if(type == json_type_object){
//				vrts_Json_String.adr 	= json_object_get_int(json_object_object_get(jvalue_rgb,"DEVICE_UNICAST_ID"));
//				vrts_Json_String.srgbID = json_object_get_int(json_object_object_get(jvalue_rgb,"SRGBID"));
//				Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRGB_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, \
//						NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 23);
//				usleep(400000);
//			}
//		}
	}
	return 1;
}

/*JsonControl(): process data json
 *Get data json
 *Set para for function send data for ble mesh
 */
void JsonControl(json_object *jobj,char *key){
	if(strcmp(key,"DATA")==0){
		enum json_type type_data_object;
		 json_object_object_foreach(vrts_Json_String.data, key_data, val_data) {
			 type_data_object= json_object_get_type(val_data);
			 switch(type_data_object){
			 case json_type_int:
				 vrts_Json_String.adr = json_object_get_int(json_object_object_get(vrts_Json_String.data, "DEVICE_UNICAST_ID"));
				break;
			 case json_type_string:
				break;
			 case json_type_array:
				json_parse_array(vrts_Json_String.data, key_data);
				break;
			 case json_type_object:
				if(strcmp(key_data,"CCT") ==0 ){
					flag_scene_cct = true;
					vrts_Json_String.cct_object = json_object_object_get(vrts_Json_String.data, "CCT");
				}
				break;
			 }
		 }
	}
	if(strcmp(vrts_Json_String.cmd,"SCAN") == 0){
		slog_print(SLOG_INFO, 1, "<provision>Provision start");
		MODE_PROVISION=true;
		pthread_create(&vrts_System_Provision,NULL, ProvisionThread, NULL);
	}
	else if(strcmp(vrts_Json_String.cmd,"STOP") == 0){
		slog_print(SLOG_INFO, 1, "<provision>Provision stop");
		MODE_PROVISION=false;
		ControlMessage(3, OUTMESSAGE_ScanStop);
		//pthread_cancel(tmp);
		flag_selectmac     = false;
		flag_getpro_info   = false;
		flag_getpro_element= false;
		flag_provision     = false;
		flag_mac           = true;
		flag_check_select_mac  = false;
		flag_done          = true;
		flag_set_type = false;

		//send mqtt
		struct json_object *object;
		object = json_object_new_object();
		json_object_object_add(object, "CMD", json_object_new_string("STOP"));
		char *rsp;
		rsp = json_object_to_json_string(object);
		mosquitto_publish(mosq, NULL, "RD_STATUS", strlen(rsp),rsp,  qos, 0);
		slog_info("(mqtt)Message_send:%s",rsp);

		//for gpio
		flag_blink = false;
		led_pin_off(gpio[LED_BLE_PIN_INDEX]);
	}
	else if(strcmp(vrts_Json_String.cmd,"UPDATE") == 0){
		FunctionPer(HCI_CMD_GATEWAY_CMD, UpdateLight_typedef, NULL16, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, 12);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"RESETNODE") == 0){
		if(flag_check_device_unicast_id){
			flag_check_device_unicast_id = false;
			for(i=0;i<arraylen;i++){
				FunctionPer(HCI_CMD_GATEWAY_CMD, ResetNode_typedef, adr_dv[i], NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, NULL16, 12);
				usleep(400000);
			}
		}
		arraylen=0;
	}
	else if(strcmp(vrts_Json_String.cmd,"ONOFF")==0){
		vrts_Json_String.onoff 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"VALUE_ONOFF"));
		FunctionPer(HCI_CMD_GATEWAY_CMD,ControlOnOff_typedef,vrts_Json_String.adr ,NULL8, vrts_Json_String.onoff, NULL16, NULL16, NULL16,\
				NULL16,NULL16, NULL16, NULL16, NULL16, 14);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"CCT") == 0){
		vrts_Json_String.cct 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"VALUE_CCT"));
		FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_typedef, (vrts_Json_String.adr), NULL8, NULL8, NULL16,Percent2ParamCCT(vrts_Json_String.cct),\
				NULL16, NULL16,NULL16, NULL16, NULL16, NULL16, 17);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DIM") == 0){
		vrts_Json_String.dim 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"VALUE_DIM"));
		FunctionPer(HCI_CMD_GATEWAY_CMD, Lightness_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, Percent2ParamDIM(vrts_Json_String.dim),\
				NULL16, NULL16, NULL16,NULL16, NULL16, NULL16, NULL16, 15);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"HSL") == 0){
		vrts_Json_String.hue 		= json_object_get_int(json_object_object_get(vrts_Json_String.data,"VALUE_H"));
		vrts_Json_String.saturation = json_object_get_int(json_object_object_get(vrts_Json_String.data,"VALUE_S"));
		vrts_Json_String.lightness 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"VALUE_L"));
		FunctionPer(HCI_CMD_GATEWAY_CMD, HSL_Set_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,\
				vrts_Json_String.lightness, vrts_Json_String.hue, vrts_Json_String.saturation, NULL16, 19 );
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDGROUP") == 0){
		check_add_or_del_group = true;
		vrts_Json_String.addgroup 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"GROUP_UNICAST_ID"));
		if(flag_check_device_unicast_id){
			flag_check_device_unicast_id = false;
			for(i=0; i<arraylen; i++){
				FunctionPer(HCI_CMD_GATEWAY_CMD, AddGroup_typedef, adr_dv[i], vrts_Json_String.addgroup , NULL8, NULL16, NULL16, NULL16, \
						NULL16,NULL16, NULL16, NULL16, NULL16, 18);
				usleep(400000);
			}
		}
		arraylen=0;
	}
	else if(strcmp(vrts_Json_String.cmd,"DELGROUP") == 0){
		check_add_or_del_group = false;
		vrts_Json_String.delgroup 	= json_object_get_int(json_object_object_get(vrts_Json_String.data, "GROUP_UNICAST_ID"));
		if(flag_check_device_unicast_id){
			flag_check_device_unicast_id = false;
			for(i=0; i<arraylen; i++){
				FunctionPer(HCI_CMD_GATEWAY_CMD, DelGroup_typedef, adr_dv[i], vrts_Json_String.delgroup, NULL8, NULL16, NULL16, NULL16,\
						NULL16,NULL16, NULL16, NULL16, NULL16, 18);
				usleep(400000);
			}
		}
		arraylen=0;
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE") == 0){
		check_add_or_del_scene = true;
		vrts_Json_String.sceneID 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		if(flag_scene_cct){
			flag_scene_cct = false;
			json_object_object_foreach(vrts_Json_String.cct_object, key_cct, val_cct){
				if(strcmp(key_cct,"DEVICE_UNICAST_ID") == 0){
					json_parse_array( vrts_Json_String.cct_object, key_cct);
				}
				if(flag_check_device_unicast_id){
					flag_check_device_unicast_id = false;
					for(i=0;i<arraylen;i++){
						FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef, adr_dv[i], NULL8, NULL8, NULL16, NULL16, \
								vrts_Json_String.sceneID, NULL16,NULL16, NULL16, NULL16, NULL16, 14);
						usleep(400000);
					}
				}
				arraylen = 0;
			}
		}
		if(flag_scene_rgbcct){
			flag_scene_rgbcct = false;
			json_object *jarray_rgb_cct = json_object_object_get(vrts_Json_String.data, "RGB_CCT");
			arraylen_rgb_cct = json_object_array_length(jarray_rgb_cct);
			for (i=0; i< arraylen_rgb_cct; i++){
				json_object *jvalue_rgb_cct = json_object_array_get_idx(jarray_rgb_cct, i);
				enum json_type type = json_object_get_type(jvalue_rgb_cct);
				if (type == json_type_object){
					vrts_Json_String.srgbID = json_object_get_int(json_object_object_get(jvalue_rgb_cct,"SRGBID"));
					vrts_Json_String.adr = json_object_get_int(json_object_object_get(jvalue_rgb_cct,"DEVICE_UNICAST_ID"));
					Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRGB_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, \
							NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 23);
					usleep(400000);
				}
			}
			arraylen_rgb_cct = 0;
		}
	}
	else if(strcmp(vrts_Json_String.cmd,"EDITSCENE") == 0){
		vrts_Json_String.sceneID 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		if(flag_scene_cct){
			flag_scene_cct = false;
			json_object_object_foreach(vrts_Json_String.cct_object, key_cct, val_cct){
				if(strcmp(key_cct,"DEVICE_UNICAST_ID") == 0){
					json_parse_array( vrts_Json_String.cct_object, key_cct);
				}
				if(flag_check_device_unicast_id){
					flag_check_device_unicast_id = false;
					for(i=0;i<arraylen;i++){
						FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef, adr_dv[i], NULL8, NULL8, NULL16, NULL16, \
								vrts_Json_String.sceneID, NULL16,NULL16, NULL16, NULL16, NULL16, 14);
						usleep(400000);
					}
				}
				arraylen = 0;
			}
		}
		if(flag_scene_rgbcct){
			flag_scene_rgbcct = false;
			json_object *jarray_rgb_cct = json_object_object_get(vrts_Json_String.data, "RGB_CCT");
			arraylen_rgb_cct = json_object_array_length(jarray_rgb_cct);
			for (i=0; i< arraylen_rgb_cct; i++){
				json_object *jvalue_rgb_cct = json_object_array_get_idx(jarray_rgb_cct, i);
				enum json_type type = json_object_get_type(jvalue_rgb_cct);
				if (type == json_type_object){
					vrts_Json_String.srgbID = json_object_get_int(json_object_object_get(jvalue_rgb_cct,"SRGBID"));
					vrts_Json_String.adr = json_object_get_int(json_object_object_get(jvalue_rgb_cct,"DEVICE_UNICAST_ID"));

					FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, vrts_Json_String.adr, NULL8, NULL8, NULL16, NULL16, \
					vrts_Json_String.sceneID, NULL16,NULL16, NULL16, NULL16, NULL16, 14);
					usleep(400000);

					Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRGB_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, \
							NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 23);
					usleep(400000);
				}
			}
			arraylen_rgb_cct = 0;
		}
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE") == 0){
		check_add_or_del_scene = false;
		vrts_Json_String.delscene 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));

		if(flag_check_device_unicast_id){
			flag_check_device_unicast_id = false;
			for(i=0; i<arraylen; i++){
				FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, adr_dv[i], NULL8, NULL8, NULL16, NULL16, \
				vrts_Json_String.delscene, NULL16,NULL16, NULL16, NULL16, NULL16, 14);
				usleep(400000);
			}
		}
		if(flag_check_device_rgb_unicast_id){
			flag_check_device_rgb_unicast_id = false;
			for(i=0; i<arraylen_rgb; i++){
				Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneRgb_vendor_typedef, adr_rgb_dv[i], NULL16, NULL8,NULL8, NULL8, NULL16,\
						NULL16, NULL16,NULL16, NULL16, NULL16, vrts_Json_String.delscene, NULL8, NULL8, NULL8, NULL8, NULL16, 23);
				usleep(400000);
			}
		}
		arraylen=0;
		arraylen_rgb=0;
	}
	else if(strcmp(vrts_Json_String.cmd,"CALLSCENE") == 0){
		vrts_Json_String.sceneID 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, NULL8, NULL8, NULL8, NULL16, NULL16, vrts_Json_String.sceneID, NULL16,NULL16, NULL16, NULL16, NULL16, 17);
		usleep(400000);
		Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8, NULL8, NULL8, NULL16,\
				NULL16, NULL16,NULL16, NULL16, NULL16, vrts_Json_String.sceneID, NULL8, NULL8, NULL8, NULL8, NULL16, 23);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"CALLMODE_RGB") == 0){
		vrts_Json_String.srgbID = json_object_get_int(json_object_object_get(vrts_Json_String.data,"SRGBID"));
		if(flag_check_device_unicast_id){
			flag_check_device_unicast_id = false;
			for(i=0; i<arraylen; i++){
				Function_Vendor(HCI_CMD_GATEWAY_CMD, CallModeRgb_vendor_typedef, adr_dv[i], NULL16, NULL8,NULL8, NULL8, NULL16,\
						NULL16, NULL16,NULL16, NULL16, NULL16, NULL16, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 23);
				usleep(400000);
			}
		}
		arraylen=0;
	}
//	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_RGB") == 0){
//		vrts_Json_String.sceneID 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
//		vrts_Json_String.srgbID 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SRGBID"));
//		if(vrts_Json_String.srgbID !=0){
//			for(i=0; i<arraylen; i++){
//				Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRGB_vendor_typedef, adr_dv[i], NULL16, NULL8, NULL8, NULL8, \
//						NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 23);
//				usleep(400000);
//			}
//		}
//	}
//	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_RGB") == 0){
//		check_add_or_del_scene = false;
//		vrts_Json_String.delscene 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
//		for(i=0; i<arraylen; i++){
//			Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneRgb_vendor_typedef, adr_dv[i], NULL16, NULL8,NULL8, NULL8, NULL16,\
//					NULL16, NULL16,NULL16, NULL16, NULL16, vrts_Json_String.delscene, NULL8, NULL8, NULL8, NULL8, NULL16, 23);
//			sleep(1);
//		}
//	}
//	else if(strcmp(vrts_Json_String.cmd,"CALLSCENE_RGB") == 0){
//		vrts_Json_String.sceneID 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
//		Function_Vendor(HCI_CMD_GATEWAY_CMD, CallSceneRgb_vendor_typedef, NULL16, NULL16, NULL8, NULL8, NULL8, NULL16,\
//				NULL16, NULL16,NULL16, NULL16, NULL16, vrts_Json_String.sceneID, NULL8, NULL8, NULL8, NULL8, NULL16, 23);
//		usleep(400000);
//	}

	else if(strcmp(vrts_Json_String.cmd,"SCAN_TYPEDEVICE") == 0){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, AskTypeDevice_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8,\
				NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"SET_TYPEDEVICE") == 0){
		vrts_Json_String.type 			= json_object_get_int(json_object_object_get(vrts_Json_String.data,"TYPE"));
		vrts_Json_String.attrubute 		= json_object_get_int(json_object_object_get(vrts_Json_String.data,"ATTRUBUTE"));
		vrts_Json_String.application 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"APPLICATION"));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SetTypeDevice_vendor_typedef, vrts_Json_String.adr, NULL16, \
				NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8,\
				vrts_Json_String.type, vrts_Json_String.attrubute, vrts_Json_String.application, NULL16, 28);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"SAVE_GW") == 0){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SaveGateway_vendor_typedef, vrts_Json_String.adr, NULL16,\
				NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_REMOTE_DC") == 0){
		vrts_Json_String.buttonid = (json_object_get_string(json_object_object_get(vrts_Json_String.data,"BUTTONID")));
		uint8_t buttonId_int;
		if(strcmp(vrts_Json_String.buttonid,"BUTTON_1")==0){
			buttonId_int =1;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_2")==0){
			buttonId_int =2;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_3")==0){
			buttonId_int =3;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_4")==0){
			buttonId_int =4;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_5")==0){
			buttonId_int =5;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_6")==0){
			buttonId_int =6;
		}
		vrts_Json_String.modeid        = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"MODEID")));
		vrts_Json_String.sceneID       = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID")));
		vrts_Json_String.srgbID        = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"SRGBID")));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRemote_DC_vendor_typedef, vrts_Json_String.adr, NULL16, buttonId_int,\
				vrts_Json_String.modeid, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID,\
				vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 31);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_REMOTE_AC") == 0){
		vrts_Json_String.buttonid     = (json_object_get_string(json_object_object_get(vrts_Json_String.data,"BUTTONID")));
		uint8_t buttonId_int;
		if(strcmp(vrts_Json_String.buttonid,"BUTTON_1")==0){
			buttonId_int =1;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_2")==0){
			buttonId_int =2;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_3")==0){
			buttonId_int =3;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_4")==0){
			buttonId_int =4;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_5")==0){
			buttonId_int =5;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_6")==0){
			buttonId_int =6;
		}
		vrts_Json_String.modeid        = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"MODEID")));
		vrts_Json_String.sceneID       = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID")));
		vrts_Json_String.srgbID        = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"SRGBID")));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForRemote_AC_vendor_typedef, vrts_Json_String.adr, NULL16, buttonId_int,\
				vrts_Json_String.modeid, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID,\
				vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 31);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_REMOTE_DC") == 0){
		vrts_Json_String.buttonid     = (json_object_get_string(json_object_object_get(vrts_Json_String.data,"BUTTONID")));
		uint8_t buttonId_int;
		if(strcmp(vrts_Json_String.buttonid,"BUTTON_1")==0){
		buttonId_int =1;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_2")==0){
		buttonId_int =2;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_3")==0){
		buttonId_int =3;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_4")==0){
		buttonId_int =4;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_5")==0){
		buttonId_int =5;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_6")==0){
		buttonId_int =6;
		}
		vrts_Json_String.modeid        = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"MODEID")));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForRemote_DC_vendor_typedef, vrts_Json_String.adr, NULL16, buttonId_int,\
				vrts_Json_String.modeid, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, \
				vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 31);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_REMOTE_AC") == 0){
		vrts_Json_String.buttonid     = (json_object_get_string(json_object_object_get(vrts_Json_String.data,"BUTTONID")));
		uint8_t buttonId_int;
		if(strcmp(vrts_Json_String.buttonid,"BUTTON_1")==0){
		buttonId_int =1;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_2")==0){
		buttonId_int =2;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_3")==0){
		buttonId_int =3;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_4")==0){
		buttonId_int =4;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_5")==0){
		buttonId_int =5;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_6")==0){
		buttonId_int =6;
		}
		vrts_Json_String.modeid = (json_object_get_int(json_object_object_get(vrts_Json_String.data,"MODEID")));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForRemote_AC_vendor_typedef, vrts_Json_String.adr, NULL16, buttonId_int,\
				vrts_Json_String.modeid, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, \
				vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 31);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_LIGHT_PIR_SENSOR") == 0){
		vrts_Json_String.type = json_object_get_int(json_object_object_get(vrts_Json_String.data,"TYPE"));
		vrts_Json_String.sceneID = json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		uint8_t condition_Sensor = 0x00;
		uint8_t condition_light = 0x00;
		if(vrts_Json_String.type == 1){
			vrts_Json_String.pir = (json_object_object_get(vrts_Json_String.data, "PIR_SENSOR"));
			vrts_Json_String.motion = json_object_get_int(json_object_object_get(vrts_Json_String.pir,"PIR"));
			vrts_Json_String.low_lux=vrts_Json_String.hight_lux=0;
		}
		else if(vrts_Json_String.type == 2){
			vrts_Json_String.pir = (json_object_object_get(vrts_Json_String.data, "PIR_SENSOR"));
			vrts_Json_String.motion = json_object_get_int(json_object_object_get(vrts_Json_String.pir,"PIR"));
			vrts_Json_String.lightsensor = (json_object_object_get(vrts_Json_String.data,"LIGHT_SENSOR"));
			vrts_Json_String.condition = json_object_get_int(json_object_object_get(vrts_Json_String.lightsensor,"CONDITION"));
			vrts_Json_String.low_lux = json_object_get_int(json_object_object_get(vrts_Json_String.lightsensor,"LOW_LUX"));
			vrts_Json_String.hight_lux = 0x0000;
			if(vrts_Json_String.condition == 4 || vrts_Json_String.condition == 3){
				condition_light = 0x01;
			}
			else if(vrts_Json_String.condition == 5 || vrts_Json_String.condition ==6){
				condition_light = 0x03;
			}
			else if(vrts_Json_String.condition == 7){
				condition_light = 0x02;
				vrts_Json_String.hight_lux = json_object_get_int(json_object_object_get(vrts_Json_String.lightsensor,"HIGHT_LUX"));
			}
		}
		condition_Sensor = (uint8_t)((vrts_Json_String.motion<<3) | (condition_light));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForSensor_LightPir_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8,\
				NULL8, condition_Sensor, vrts_Json_String.low_lux/10, vrts_Json_String.hight_lux/10, NULL8, NULL8, vrts_Json_String.sceneID , NULL16,\
				NULL8, NULL8, NULL8, NULL8, NULL16, 23);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_LIGHT_SENSOR") == 0){
		//update

	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_SCREEN_TOUCH") == 0){
		vrts_Json_String.buttonid     = (json_object_get_string(json_object_object_get(vrts_Json_String.data,"BUTTONID")));
		uint8_t buttonId_int;
		if(strcmp(vrts_Json_String.buttonid,"BUTTON_1")==0){
		buttonId_int =1;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_2")==0){
		buttonId_int =2;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_3")==0){
		buttonId_int =3;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_4")==0){
		buttonId_int =4;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_5")==0){
		buttonId_int =5;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_6")==0){
		buttonId_int =6;
		}
		vrts_Json_String.sceneID 	= 	json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		vrts_Json_String.srgbID		=	json_object_get_int(json_object_object_get(vrts_Json_String.data,"SRGBID"));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForScreenT_vendor_typedef, vrts_Json_String.adr, NULL16, buttonId_int, NULL8, NULL8, NULL16, \
				NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 21);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_SCREEN_TOUCH") == 0){
		vrts_Json_String.buttonid     = (json_object_get_string(json_object_object_get(vrts_Json_String.data,"BUTTONID")));
		uint8_t buttonId_int;
		if(strcmp(vrts_Json_String.buttonid,"BUTTON_1")==0){
		buttonId_int =1;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_2")==0){
		buttonId_int =2;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_3")==0){
		buttonId_int =3;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_4")==0){
		buttonId_int =4;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_5")==0){
		buttonId_int =5;
		}
		else if(strcmp(vrts_Json_String.buttonid,"BUTTON_6")==0){
		buttonId_int =6;
		}
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForScreenT_vendor_typedef, vrts_Json_String.adr, NULL16, \
				buttonId_int, NULL8, NULL8 , NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 18);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"SET_BALANCE_LIGHT_SENSOR") == 0){
		vrts_Json_String.lightsensor = json_object_object_get(vrts_Json_String.data,"LIGHT_SENSOR");
		uint16_t lightness_int 	= json_object_get_int(json_object_object_get(vrts_Json_String.lightsensor,"LIGHTNESS"));
		uint16_t adr_des_int 	= json_object_get_int(json_object_object_get(vrts_Json_String.lightsensor,"DESTINANTION"));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, Balance_Lightness_vendor_typedef, vrts_Json_String.adr, adr_des_int, NULL8, NULL8, NULL8, lightness_int,\
				NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 21);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DEL_BALANCE_LIGHT_SENSOR") == 0){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelBalance_Lightness_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, NULL16, \
				NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_LIGHT_SENSOR") == 0 ){
		vrts_Json_String.sceneID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		uint16_t parCondition = 1280;
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForSensor_Light_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, \
					parCondition, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, NULL8, NULL8, NULL8, NULL8, NULL16, 19);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_PIR_SENSOR") == 0 ){
		vrts_Json_String.sceneID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		vrts_Json_String.srgbID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SRGBID"));
		vrts_Json_String.pir 			= json_object_object_get(vrts_Json_String.data,"PIR_SENSOR");
		vrts_Json_String.motion 		= (json_object_get_int(json_object_object_get(vrts_Json_String.pir,"PIR")));
		if(vrts_Json_String.motion == 1){
			uint16_t parCondition = 1;
			Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForSensor_Pir_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, parCondition, NULL16, NULL16,\
					NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 26);
			usleep(400000);
		}
		if(vrts_Json_String.motion == 0){
			uint16_t parCondition = 2;
			Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForSensor_Pir_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, parCondition, NULL16, NULL16,\
					NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 26);
			usleep(400000);
		}
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_PIR_SENSOR") == 0 ){
		vrts_Json_String.sceneID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		uint16_t parCondition = 1280;
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForSensor_Pir_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, \
					parCondition, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, NULL8, NULL8, NULL8, NULL8, NULL16, 19);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_LIGHT_PIR_SENSOR") ==0 ){
		vrts_Json_String.sceneID = json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		 uint16_t parCondition = 1280;
		 Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForSensor_LightPir_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, \
	 					parCondition, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, NULL8, NULL8, NULL8, NULL8, NULL16, 19);
		 usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_DOOR_SENSOR") == 0 ){
		vrts_Json_String.sceneID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		vrts_Json_String.srgbID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SRGBID"));
		vrts_Json_String.doorsensor 	= json_object_object_get(vrts_Json_String.data,"DOOR_SENSOR");
		vrts_Json_String.door_value 	= json_object_get_int(json_object_object_get(vrts_Json_String.doorsensor,"DOOR"));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SceneForDoorSensor_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, vrts_Json_String.door_value, NULL16, \
				NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, vrts_Json_String.srgbID, NULL8, NULL8, NULL8, NULL16, 21);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_DOOR_SENSOR")  == 0 ){
		vrts_Json_String.sceneID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForDoorSensor_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, \
				NULL16, NULL16, vrts_Json_String.sceneID, vrts_Json_String.sceneID, NULL8, NULL8, NULL8, NULL8, NULL16, 19);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"CONTROL_SWITCH4") == 0){
		vrts_Json_String.switch4 	= json_object_object_get(vrts_Json_String.data, "SWITCH_STATUS");
		uint8_t numRelay= 0;
		uint8_t valRelay= 0;
		 json_object_object_foreach(vrts_Json_String.switch4, key_switch4, val_switch4) {
			 if(strcmp(key_switch4,"RELAY1") == 0){
				 numRelay =1;
				 valRelay = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY1")) & 0xFF;
			 }
			 else if(strcmp(key_switch4,"RELAY2") == 0){
				 numRelay =2;
				 valRelay = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY2")) & 0xFF;
			 }
			 else if(strcmp(key_switch4,"RELAY3") == 0){
				 numRelay =3;
				 valRelay = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY3")) & 0xFF;
			 }
			 else if(strcmp(key_switch4,"RELAY4") == 0){
				 numRelay =4;
				 valRelay = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY4")) & 0xFF;
			 }
		 }
		Function_Vendor(HCI_CMD_GATEWAY_CMD, ControlSwitch4_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, NULL16, (numRelay<<8 | valRelay), NULL16, \
				NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 23);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"ADDSCENE_SWITCH4") == 0){
		vrts_Json_String.sceneID 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		vrts_Json_String.switch4 	= json_object_object_get(vrts_Json_String.data, "SWITCH_STATUS");
		uint8_t sw1 = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY1")) & 0xFF;
		uint8_t sw2 = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY2")) & 0xFF;
		uint8_t sw3 = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY3")) & 0xFF;
		uint8_t sw4 = json_object_get_int(json_object_object_get(vrts_Json_String.switch4,"RELAY4")) & 0xFF;
		Function_Vendor(HCI_CMD_GATEWAY_CMD, SetSceneForSwitch4_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, NULL8, NULL16, (sw1<<8 | sw2), (sw3<<8 | sw4),
				NULL16, NULL16, vrts_Json_String.sceneID, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 23);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd,"DELSCENE_SWITCH4") == 0){
		vrts_Json_String.sceneID 	 	= json_object_get_int(json_object_object_get(vrts_Json_String.data,"SCENEID"));
		Function_Vendor(HCI_CMD_GATEWAY_CMD, DelSceneForSwitch4_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, \
				NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, vrts_Json_String.sceneID, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 19);
		usleep(400000);
	}
	else if(strcmp(vrts_Json_String.cmd, "ASK_PM_SENSOR") == 0){
		Function_Vendor(HCI_CMD_GATEWAY_CMD, AskPm_vendor_typedef, vrts_Json_String.adr, NULL16, NULL8, NULL8, \
				NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
		usleep(400000);
	}
	vrts_Json_String.cmd = "\0";
}

/*
 * Json_Parse(): Get key value of data json
 */
void Json_Parse(json_object * jobj)
{
	enum json_type type;
		 json_object_object_foreach(jobj, key, val) {
			 type= json_object_get_type(val);
			 switch(type){
			 case json_type_int:
				break;
			 case json_type_string:
				vrts_Json_String.cmd 	= (json_object_get_string(json_object_object_get(jobj,"CMD")));
				break;
			 case json_type_array:
				json_parse_array(jobj, key);
				break;
			 case json_type_object:
				vrts_Json_String.data = json_object_object_get(jobj,"DATA");
				break;
			 }
		 }
		 JsonControl(jobj, key);
}

/*
 * creat a object json with a key and a value (type value: int, string, object)
 */
void add_component_to_obj(json_object *j, void* com){
    json_component *dt = (json_component*)com;
    switch (dt->type){
    case json_type_int:
    	json_object_object_add(j, dt->key, json_object_new_int(dt->value));
    	break;
    case json_type_string:
    	json_object_object_add(j, dt->key, json_object_new_string(dt->value));
    	break;
    case json_type_array:
    	//json_object_object_add(j, dt->key, json_object_new_array(dt->value));
    	break;
    case json_type_boolean:
    	json_object_object_add(j, dt->key, json_object_new_boolean(dt->value));
    	break;
    case json_type_object:
    	json_object_object_add(j, dt->key,(dt->value));
    	break;
    }
    return;
}

/*
 * creat object json, include json_component
 * Check mqtt push json
 */
json_object* create_json_obj_from(void (*modelFunc)(json_object*, void*), int num_of, send_mqtt mqtt,...){
    va_list args_list;
    json_object *jobj = json_object_new_object();
    typedef void *com;

    va_start(args_list, num_of);
    int i;
    for(i = 0; i< num_of; i++){
        add_component_to_obj(jobj, va_arg(args_list, com));
    }
    va_end(args_list);
    if(mqtt == mqtt_push){
		char *str = json_object_to_json_string(jobj);
		mosquitto_publish(mosq, NULL, TP_STATUS, strlen(str), str, qos, 0);
		slog_info("(mqtt)Message_send:%s",str);
    }
    return jobj;

}
