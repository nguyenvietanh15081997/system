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
typedef struct jsonstring
{
	uint16_t adr;
	uint8_t onoff;
	uint16_t cct;
	uint16_t dim;
	uint8_t addgroup;
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
} jsonstring;
extern jsonstring vrts_Json_String;
extern char flagSecond;

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
	resetnode_enum	=18
} defineCmd;
extern defineCmd flagDefineCmd;

void JsonControl(char *key);
void Json_Parse(json_object * jobj);
void CreatJson(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,uint16_t par1, uint16_t par2);
void CreatJson_TypeDev(uint8_t *topic, uint8_t *objectJsonAdr, uint8_t *objectJsonMain, uint8_t *objectJsonSub, uint8_t *objectJsonPower, uint8_t parAdr, uint8_t parMain, uint8_t parSub, uint8_t parPower);

#ifdef __cplusplus
}
#endif

#endif
