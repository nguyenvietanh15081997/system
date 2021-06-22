#include "../GatewayManager/Linkerlist.h"
#include "../GatewayManager/JsonProcess.h"
#include "../GatewayManager/ShareMessage.h"
#include "../GatewayManager/slog.h"

#include <time.h>

clock_t start_t,end_t,total_t;
//void Clock_Time(){
//	start_t = clock();
//	do{
//		end_t= clock();
//		total_t = (double)(end_t-start_t)/CLOCKS_PER_SEC;
//	}
//	while(total_t != 40000000);
//}
typedef struct buffer_wait *vrts_buff;

vrts_buff head = NULL;
bool flag_check_rsp = true;
long long timeoutRsp = 0;

pthread_t vrpth_LinkerList= PTHREAD_MUTEX_INITIALIZER;

vrts_buff Create_buffer(char * message){
	//puts("Create buffer save msg");
	int length = strlen(message);
	char *create_buffer = (char *)malloc(length + 1);
	strcpy(create_buffer,message);
	*(create_buffer + length) = '\0';
	vrts_buff newdata;
	newdata = (vrts_buff)malloc(sizeof(struct buffer_wait));
	newdata->message = create_buffer;
	newdata->next = NULL;
	//free(create_buffer);
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

bool mode_setup = true;
uint8_t time_Send_setup = 0;
bool get_start_time = true;
bool flag_new_send = true;

void *LinkerList_Thread(void *argv)
{
	while(1){
		while(pthread_mutex_lock(&vrpth_LinkerList) != 0){

		}
		if(head != NULL){
			if(mode_setup){
				//pthread_mutex_lock(&vrpth_SHAREMESS_FlagCheckRsp);
				printf("\tflag_check = %d\n",flag_check_rsp);
				flag_check_rsp = 0;
				get_start_time = true;
				while(!flag_check_rsp){
					if(get_start_time){
						get_start_time = false;
						start_t =clock();
						puts("GET START TIME");
					}
					end_t = clock();
					if(flag_check_rsp){
						printf("ABOUT TIME = %f\n",(double)(end_t-start_t));
					}
					//pthread_mutex_unlock(&vrpth_SHAREMESS_FlagCheckRsp);
					//printf("\tflag_check = %d\n",flag_check_rsp);
					//slog_print(SLOG_INFO, 1, "timeout= %d",timeoutRsp);
					//printf("%d\n",timeoutRsp);
					if(((double)(end_t - start_t)) >= 450000){
						get_start_time = true;
						puts("PROCESS JSON AND SEND CMD");
						struct json_obj* jobj = json_tokener_parse(head->message);
						Json_Parse(jobj);
						time_Send_setup++;
						if(time_Send_setup == 3){
							puts("3 TIME SEND");
							time_Send_setup = 0;
							//pthread_mutex_lock(&vrpth_SHAREMESS_FlagCheckRsp);
							flag_check_rsp = true;
							//pthread_mutex_unlock(&vrpth_SHAREMESS_FlagCheckRsp);
						}
					}
					//pthread_mutex_lock(&vrpth_SHAREMESS_FlagCheckRsp);
					usleep(1000);
				}
				head = DellHead(head);
				printf("\tDELETE LINKER LIST\n");
				ShowLL(head);
			}
		}
		pthread_mutex_unlock(&vrpth_LinkerList);
		usleep(1000);
	}
	return NULL;
}


