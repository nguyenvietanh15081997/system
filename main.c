/*
 ============================================================================
 Name        : GatewayManager.c
 Author      : DuanLC
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "GatewayManager/GateInterface.h"
//#include "GatewayManager/cli.h"

pthread_t vrts_System_ThreadGWIF;
pthread_t vrts_System_TestSend;


static uint8_t unicast1,unicast2;
static uint8_t groupadd;
static uint8_t groupdel;
static uint8_t groupcontrol;

unsigned int timescan;

char selectmode[50];
char mode1[]="checknode";
char mode2[]="selectnode";
char mode3[]="control";
char mode4[]="addgroup";
char mode5[]="deletegroup";
char mode6[]="kichout";
char mode7[]="provision";
char Help[] ="help";

const static uint8_t OUTMESSAGE_ScanStart[3] 		= {0xE9, 0xFF, 0x00};
 uint8_t OUTMESSAGE_Check[12]               = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0xff, 0xff, 0x82, 0x4b};
 uint8_t OUTMESSAGE_SelectNodeSend1[16]     = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x80, 0x29, 0xff, 0xff, 0x00, 0x10};
 uint8_t OUTMESSAGE_SelectNodeSend2[12]     = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x44};
 uint8_t OUTMESSAGE_Addgroup[18]            = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x80, 0x1b, 0xff, 0xff, 0x00, 0xc0, 0x00, 0x10};
 uint8_t OUTMESSAGE_Delgroup[18]            = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x80, 0x1c, 0xff, 0xff, 0x00, 0xc0, 0x00, 0x10};
 uint8_t OUTMESSAGE_ControlGroup[14]        = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0xc0, 0x82, 0x02, 0x00, 0x00};
 uint8_t OUTMESSAGE_KichOut[12]             = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0xff, 0xff, 0x80, 0x49};



void controlmessage(uint16_t lengthmessage,uint8_t *Message){
	pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
	vrb_SHAREMESS_Send2GatewayAvailabe = true;
	vrui_SHAREMESS_Send2GatewayLength = lengthmessage;
	memcpy(vrsc_SHAREMESS_Send2GatewayMessage, Message, vrui_SHAREMESS_Send2GatewayLength);
	pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
}
void checknode()
{
	controlmessage(12,OUTMESSAGE_Check);
	printf("Check node\n");
}
void Selectnode()
{
	char Selectnode[]= "Select node";
	controlmessage(11,Selectnode);
	//controlmessage(3,downline);
}

 /*
 * Ham thu nghiem Scan Network de xu ly ban tin ve tu Gateway
 * TODO: Hoan thien va loai bo
 */
void *TestSendThread (void *vargp){
while(1){
	gets(selectmode);
	if (strcmp(selectmode, mode1) == 0)
	{
		flag_mode=0;
		controlmessage(12,OUTMESSAGE_Check);
	}
	else if(strcmp(selectmode, mode2) == 0)
	{
		flag_mode=0;
		printf ("Select node:\n");
		scanf("%x %x",&unicast1,&unicast2);
		OUTMESSAGE_SelectNodeSend1[8]=unicast1;
		OUTMESSAGE_SelectNodeSend1[9]=unicast2;
		OUTMESSAGE_SelectNodeSend1[12]=unicast1;
		OUTMESSAGE_SelectNodeSend1[13]=unicast2;
		OUTMESSAGE_SelectNodeSend2[8]=unicast1;
		OUTMESSAGE_SelectNodeSend2[9]=unicast2;
		controlmessage(16, OUTMESSAGE_SelectNodeSend1);
		sleep(1);
		if(flag_selectnode1==1)
		{
			flag_selectnode1=0;
			controlmessage(12, OUTMESSAGE_SelectNodeSend2);
			sleep(1);
		}
		if(flag_selectnode2==1)
		{
			flag_selectnode2=0;
			printf("Selected:%x %x\n",OUTMESSAGE_SelectNodeSend1[8],OUTMESSAGE_SelectNodeSend1[9]);
		}
	}
	else if(strcmp(selectmode, mode4) == 0){
		flag_mode=0;
		OUTMESSAGE_Addgroup[8]=unicast1;
		OUTMESSAGE_Addgroup[9]=unicast2;
		OUTMESSAGE_Addgroup[12]=unicast1;
		OUTMESSAGE_Addgroup[13]=unicast2;
		printf ("Add into group: ");
		scanf("%x",&groupadd);
        OUTMESSAGE_Addgroup[14]=groupadd;
        controlmessage(18, OUTMESSAGE_Addgroup);
		printf("Add %x %x into group %x\n",unicast1,unicast2,groupadd);
		sleep(1);
	}
	else if(strcmp(selectmode, mode5) == 0){
		flag_mode=0;
		OUTMESSAGE_Delgroup[8]=unicast1;
		OUTMESSAGE_Delgroup[9]=unicast2;
		OUTMESSAGE_Delgroup[12]=unicast1;
		OUTMESSAGE_Delgroup[13]=unicast2;
		printf ("Get out group: ");
		scanf("%x",&groupdel);
		OUTMESSAGE_Delgroup[14]=groupdel;
		controlmessage(18, OUTMESSAGE_Delgroup);
		printf("Delete %x %x get out group %x\n",unicast1,unicast2,groupdel);
		sleep(1);
	}
	else if (strcmp(selectmode, mode3) == 0){
		flag_mode=0;
		printf ("Control group: ");scanf("%x",&groupcontrol);
		OUTMESSAGE_ControlGroup[8]= groupcontrol;
		controlmessage(14, OUTMESSAGE_ControlGroup);
		printf("OFF\n");
		sleep(2);
		OUTMESSAGE_ControlGroup[12]=1;
		controlmessage(14, OUTMESSAGE_ControlGroup);
		printf("ON\n");
		sleep(2);
		OUTMESSAGE_ControlGroup[12]=0;
		controlmessage(14, OUTMESSAGE_ControlGroup);
		printf("OFF\n");
		sleep(2);
		OUTMESSAGE_ControlGroup[12]=1;
		controlmessage(14, OUTMESSAGE_ControlGroup);
		printf("ON\n");
		sleep(2);
	}
	else if (strcmp(selectmode, mode6) == 0){
		flag_mode=0;
		OUTMESSAGE_KichOut[8]=unicast1;
		OUTMESSAGE_KichOut[9]=unicast2;
		controlmessage(12, OUTMESSAGE_KichOut);
	}
	else if(strcmp(selectmode, mode7) == 0){
		timescan=11;
		while(timescan!=0){
			if(((flag_scan == 1) && (flag_done == 1)) || (timeout == 32000)){
				flag_scan=0;
				timeout=0;
				sleep(3);
				controlmessage(3, OUTMESSAGE_ScanStart);
				printf ("scan\n");
				timescan--;
			}
			if((flag_selectmac==1) && (flag_mac==1)){
				flag_selectmac=0;
				flag_mac=0;
				controlmessage(9, OUTMESSAGE_MACSelect);
				printf("Select mac\n");
				sleep(2);

				controlmessage(3, OUTMESSAGE_GetPro);
				printf ("getpro\n");
				sleep(1);
			}
			if(flag_getpro_info == 1 && flag_getpro_element == 1){
				flag_getpro_info=0;
				flag_getpro_element=0;
				controlmessage(28, OUTMESSAGE_Provision);
				printf ("provision\n");
			}
			if(flag_provision == 1){
				flag_provision=0;
				flag_done=0;
				sleep(2);
				controlmessage(22, OUTMESSAGE_BindingALl);
				printf ("binding all\n");
			}
		}
	}
	else if(strcmp(selectmode, Help) == 0)
	{
		printf("%s: Add device into mesh\n",mode7);
	    printf("%s: check node in mesh\n",mode1);
	    printf("%s: Select node\n",mode2);
	    printf("%s: Control device in group\n",mode3);
	    printf("%s: Add node into group\n",mode4);
	    printf("%s: Delete node out group\n",mode5);
	    printf("%s: Kich out node get out mesh\n",mode6);
	    //puts(Help);
	}
	//else puts("Not support");
}
	return NULL;
}



/*
 * Ham xu ly chinh. Thuc thi:
 * - Goi cac tac vu tuong ung cua chuong trinh
 * PENDING: Tiep tuc bo sung them cac Task khac
 */
int main(void) {
    printf ("------SELECT MODE TO CONTROL------\n");

	pthread_create(&vrts_System_ThreadGWIF, NULL, GWINF_Thread, NULL);
	pthread_create(&vrts_System_TestSend, NULL, TestSendThread, NULL);
	pthread_join(vrts_System_ThreadGWIF, NULL);
	pthread_join(vrts_System_TestSend, NULL);

	while(1){
	}
	return EXIT_SUCCESS;
}
