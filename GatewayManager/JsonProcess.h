#ifndef GATEWAYMANAGER_JSONPROCESS_H_
#define GATEWAYMANAGER_JSONPROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <json-c/json.h>
#include <stdlib.h>

extern uint16_t valueJsonRec[2];

void Json_Parse(json_object * jobj);
void CreatJson(struct json_object * topic,uint8_t * objectJson,uint8_t par1, uint8_t par2);

#ifdef __cplusplus
}
#endif

#endif