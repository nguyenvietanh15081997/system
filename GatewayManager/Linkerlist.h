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
 *Struct of node of linker
 *poniter message: message mqtt send to
 *buffer_wait * next: link to next node
 */
struct buffer_wait{
	char *message;
	struct buffer_wait * next;
};

typedef struct buffer_wait *vrts_buff;
//extern bool flag_process_available;

extern vrts_buff head;

/*
 *Create node save adr data from db
 *
 *para message: message of mqtt send to
 *return a node of linker list
 */
vrts_buff Create_buffer(char * message);

/*
 *Add new data to database into tail of linkerlist
 *
 *para head head of linker list
 *para message data of node in linker list
 *return head of linker list
 */
vrts_buff AddTail(vrts_buff head, char * message);

/*
 *Del old data to head linker list
 *
 *para head: head of linker list
 *return head of linker list
 */
vrts_buff DellHead(vrts_buff head);

/*
 *Init head of linkerlist
 *
 *para NULL
 *return head of linker list
 */
vrts_buff InitHead();

/*
 *Get data to buffer 
 */
void Process(vrts_buff head);

#ifdef __cplusplus
}
#endif

#endif /* GATEWAYMANAGER_MQTT_H_ */
