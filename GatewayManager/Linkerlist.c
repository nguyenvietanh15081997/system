#include "../GatewayManager/Linkerlist.h"
#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/ShareMessage.h"
#include "../GatewayManager/slog.h"

#include <time.h>

clock_t start, end;

typedef struct buffer_wait *vrts_buff;

vrts_buff head = NULL;
bool flag_check_rsp = true;
int timeoutRsp = 0;

pthread_t vrpth_LinkerList= PTHREAD_MUTEX_INITIALIZER;

vrts_buff Create_buffer(char * message){
	//puts("Create buffer save msg");
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
		free(head->message);
		free(head);
	}
	return newhead;
}

vrts_buff InitHead(){
	vrts_buff head;
	head = NULL;
	return head;
}
void ShowLL(vrts_buff head){
	if(head == NULL){
		printf("linkerlist NULL");
	}
	else if(head->next == NULL){
		printf("\tData in linkerlist: %s\n",head->message);
	}
	else {
		vrts_buff p = head;
		for(p=head;p->next != NULL; p= p->next){
			printf("\tData in linkerlist: %s\n",p->message);
		}
		printf("\tData in linkerlist: %s\n",p->message);
	}
}

void *LinkerList_Thread(void *argv)
{
	while(1){
		//pthread_mutex_trylock(&vrpth_LinkerList);
		//puts("into process linker list");
		if(head != NULL){
			if(head->next == NULL){
				timeoutRsp = 0;
				struct json_obj* jobj = json_tokener_parse(head->message);
				Json_Parse(jobj);
				head = DellHead(head);
			}
			else {
				timeoutRsp++;
				slog_print(SLOG_INFO, 1, "timeout= %d",timeoutRsp);
				if((flag_check_rsp == true) || (timeoutRsp == 64)){
					//slog_print(SLOG_INFO, 1, "timeout= %d",timeoutRsp);
					//puts("done rsp");
					flag_check_rsp = false;
					timeoutRsp =0;
					//slog_print(SLOG_INFO, 1, "timeout= %d",timeoutRsp);
					struct json_obj* jobj = json_tokener_parse(head->message);
					Json_Parse(jobj);
					head = DellHead(head);
				}
			}
		}
		//pthread_mutex_unlock(&vrpth_LinkerList);
	}
	return NULL;
}


