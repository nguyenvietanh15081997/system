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

extern uint16_t numObject;
extern uint16_t valueObject[20];
extern uint16_t adr_dv[100], j;
typedef struct jsonstring
{
	uint16_t id;
	uint16_t adr;
	uint8_t onoff;
	uint16_t cct;
	uint16_t dim;
	uint16_t addgroup;
	uint16_t addscene;
	uint16_t callscene;
	uint16_t delscene;
	uint16_t delgroup;
	uint16_t hours;
	uint16_t minutes;
	uint16_t seconds;
	uint16_t hue;
	uint16_t saturation;
	uint16_t lightness;
	uint8_t resetnode;
	uint8_t start;
	uint8_t stop;
	uint8_t update;
	uint16_t header;
	uint8_t stt;
	uint16_t condition;
	uint8_t buttonid;
	uint8_t modeid;
	uint16_t low_lux;
	uint16_t hight_lux;
	uint16_t action;
	uint16_t appID;
	uint8_t srgbID;
	uint16_t sceneID;
	uint8_t *cmd;

	uint8_t sceneforremote;
	uint8_t sceneforsensor;

	uint8_t setsceneRGB;
	uint8_t callsceneRGB;
	uint8_t callmodeRGB;
	uint8_t delsceneRGB;

	uint8_t savegateway;
	uint8_t settypedevice;
	uint8_t typedevicescan;
	uint8_t type;
	uint8_t attrubute;
	uint8_t application;
} jsonstring;
extern jsonstring vrts_Json_String;
extern char flagSecond;

extern _Bool arr_json, flag_addscene, flag_delscene;
extern int i, arraylen, lst_count_id;

typedef enum{
	onoff_enum		=1,
	cct_enum		=2,
	dim_enum		=3,
	addgroup_enum	=4,
	delgroup_enum	=5,
	addscene_enum	=6,
	callscene_enum	=7,
	delscene_enum	=8,
	hours_enum		=9,
	minutes_enum	=10,
	seconds_enum	=11,
	hue_eum			=12,
	saturation_enum	=13,
	lightness_enum	=14,
	start_enum		=15,
	stop_enum		=16,
	update_enum		=17,
	resetnode_enum	=18,
	setsceneforremote_enum=19,
	buttonid_enum         =20,
	modeid_enum           =21,

} defineCmd;
extern defineCmd flagDefineCmd;

void JsonControl(json_object *jobj,char *key);

void json_value(json_object *jobj);

int json_parse_array( json_object *jobj, char *key);

void Json_Parse(json_object * jobj);

void CreatJson(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,uint16_t par1, uint16_t par2);

void CreatJsonString(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,char * par1, uint16_t par2);

void CreatJsonString_2(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue , uint16_t par1, char * par2);

void CreatJson_TypeDev(uint8_t *topic, uint8_t *objectJsonAdr, uint8_t *objectJsonType, uint8_t *objectJsonAttrubute, \
		uint8_t *objectJsonApplication,uint16_t parAdr, uint16_t parType, uint16_t parAttrubute, uint16_t parApplication);

//void CreatJson_New_TypeDev(uint8_t *topic,uint8_t * key1, uint8_t * key2, uint8_t * key3, uint8_t * key4,\
//		 uint16_t value1, uint16_t value2,uint8_t *value3);

void CreatJson_New_TypeDev(uint8_t *topic,uint8_t * key1, uint8_t * key2, uint8_t * key3, uint8_t * key4,\
		uint8_t * key5,uint8_t * key6,uint8_t * key7,uint8_t * key8,uint16_t value1, uint8_t *value2,uint8_t *value3,\
		 uint8_t *value4, uint8_t *value5,uint16_t value6,uint8_t *value7);

#ifdef __cplusplus
}
#endif

#endif
