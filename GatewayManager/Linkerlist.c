#include "../GatewayManager/Linkerlist.h"
#include "../GatewayManager/JsonProcess.h"


typedef struct buffer_wait *vrts_buff;

vrts_buff head = NULL;

vrts_buff Create_buffer(char * message){
	puts("Create buffer save msg");
	int length = strlen(message);
	char *create_buffer = (char *)malloc(length);
	strcpy(create_buffer,message);
	vrts_buff newdata;
	newdata = (vrts_buff)malloc(sizeof(struct buffer_wait));
	newdata->message = create_buffer;
	newdata->next = NULL;
	return newdata;
}

vrts_buff AddTail(vrts_buff headA, char * message)
{
	vrts_buff temp,p;
	temp=Create_buffer(message);
	if(head == NULL){
		head = temp;
	}
	else
	{
		p=head;
		while(p->next != NULL){
			p=p->next;
		}
		p->next = temp;
	}
	return head;
}

vrts_buff DellHead(vrts_buff head){
	vrts_buff newhead;
	if(head != NULL){
		newhead = head->next;
		free(head);
	}
	return newhead;
}

vrts_buff InitHead(){
	vrts_buff head;
	head = NULL;
	return head;
}

void ProcessNode(vrts_buff head)
{
	while(head != NULL){
		if(head->next == NULL){
			struct json_obj* jobj = json_tokener_parse(head->message);
			Json_Parse(jobj);
		}
		else {
			vrts_buff p=head;
			while(p->next != NULL ){
				struct json_obj* jobj = json_tokener_parse(p->message);
				Json_Parse(jobj);
				p=p->next;
			}
			struct json_obj* jobj = json_tokener_parse(p->message);
			Json_Parse(jobj);
		}
	}
}


