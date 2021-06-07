#include "../GatewayManager/Linkerlist.h"


typedef struct buffer_wait *vrts_buff;


vrts_buff Create_buffer(char * adr_buf){
	vrts_buff newdata;
	newdata = (vrts_buff)malloc(sizeof(struct buffer_wait));
	newdata->adr_buf = adr_buf;
	newdata->next = NULL;
	return newdata;
}

vrts_buff AddTail(vrts_buff head, char * adr_buf)
{
	vrts_buff temp,p;
	temp=Create_buffer(adr_buf);
	if(head == NULL){
		head = temp;
	}
	else{
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

