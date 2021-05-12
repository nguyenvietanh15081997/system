/*
 * JsonProcess.h process json data
 * Transmit data format json
 *
 */
#ifndef GATEWAYMANAGER_JSONPROCESS_H_
#define GATEWAYMANAGER_JSONPROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//extern uint16_t adr_dv[100];
typedef struct jsonstring
{
	uint16_t adr;//
	uint8_t onoff;//
	uint16_t cct;//
	uint16_t dim;//

	uint16_t addgroup;
	uint16_t delgroup;
	uint16_t delscene;
	uint16_t hours;
	uint16_t minutes;
	uint16_t seconds;
	uint16_t hue;//
	uint16_t saturation;//
	uint16_t lightness;//
	uint8_t resetnode;

	uint16_t condition;//
	uint8_t *buttonid;//
	uint8_t modeid;//
	uint16_t low_lux;//
	uint16_t hight_lux;//
	uint8_t door_value;
	uint8_t motion;//
	uint16_t action;
	uint16_t appID;//
	uint8_t srgbID;//
	uint16_t sceneID;//

	uint8_t type;//
	uint8_t attrubute;//
	uint8_t application;//

	json_object *lightsensor;
	json_object *pir;
	json_object *doorsensor;
	json_object *switch4;

	json_object *data;//
	uint8_t *cmd;//

} jsonstring;
extern jsonstring vrts_Json_String;

extern _Bool flag_addscene, flag_delscene;
extern int i, arraylen;

void JsonControl(json_object *jobj,char *key);

int json_parse_array( json_object *jobj, char *key);

void Json_Parse(json_object * jobj);

typedef enum {
	mqtt_push,
	mqtt_dont_push
}send_mqtt;

/*
 * ingredient of a json data
 */
typedef struct json_component{
	uint8_t *key;
	void  *value;
	json_type type;
}json_component;

/*
 * add component to object json
 */
void add_component_to_obj(json_object *j, void* com);

/*
    @feature: create json_object from model
    @params:
        *add_smt_to_obj: point to callback function
        num_of: number of model
        ... : model
    @return: a point to json_object
*/
extern json_object* create_json_obj_from(void (*add_smt_to_obj)(json_object*, void*), int num_of,send_mqtt mqtt, ...);


#ifdef __cplusplus
}
#endif

#endif
