#ifndef GATEWAYMANAGER_LINKERLIST_H_
#define GATEWAYMANAGER_LINKERLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> 
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/*
 *Struct buffer save adr cmd control to db
 */
struct buffer_wait{
	char *adr_buf;
	struct buffer_wait * next;
};
typedef struct buffer_wait *vrts_buff;

/*
 *Create node save adr data from db
 */
vrts_buff Create_buffer(char * adr_buf);

/*
 *Add new data to db into tail of linkerlist
 */
vrts_buff AddTail(vrts_buff head, char * adr_buf);

/*
 *Del old data 
 */
vrts_buff DellHead(vrts_buff head);

/*
 *Init head of linkerlist
 */
vrts_buff InitHead();

#ifdef __cplusplus
}
#endif

#endif /* GATEWAYMANAGER_MQTT_H_ */