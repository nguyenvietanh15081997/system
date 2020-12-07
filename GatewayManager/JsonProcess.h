#ifndef GATEWAYMANAGER_JSONPROCESS_H_
#define GATEWAYMANAGER_JSONPROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <json-c/json.h>
#include <stdlib.h>

extern uint16_t valueJsonRec[2];

void Json_Parse(json_object * jobj);
void CreatJson(uint8_t *topic,uint8_t * objectJsonAdr,uint8_t *objectJsonValue ,uint16_t par1, uint16_t par2);

#ifdef __cplusplus
}
#endif

#endif