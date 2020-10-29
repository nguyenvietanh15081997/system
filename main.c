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

pthread_t vrts_System_ThreadGWIF;
pthread_t vrts_System_TestSend;

//const static uint8_t OUTMESSAGE_ScanStart[3] 		= {0xE9, 0xFF, 0x00};
static uint8_t unicast1,unicast2;
static uint8_t groupadd;
static uint8_t groupdel;
static uint8_t groupcontrol;
static uint8_t mode;

const static uint8_t OUTMESSAGE_Check[12]   = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0xff, 0xff, 0x82, 0x4b};
 uint8_t OUTMESSAGE_SelectNodeSend1[16]     = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x80, 0x29, 0xff, 0xff, 0x00, 0x10};
 uint8_t OUTMESSAGE_SelectNodeSend2[12]     = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x44};
 uint8_t OUTMESSAGE_Addgroup[18]            = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x80, 0x1b, 0xff, 0xff, 0x00, 0xc0, 0x00, 0x10};
 uint8_t OUTMESSAGE_Delgroup[18]            = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x80, 0x1c, 0xff, 0xff, 0x00, 0xc0, 0x00, 0x10};
 uint8_t OUTMESSAGE_ControlGroup[14]        = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0xc0, 0x82, 0x02, 0x00, 0x00};
 /*
 * Ham thu nghiem Scan Network de xu ly ban tin ve tu Gateway
 * TODO: Hoan thien va loai bo
 */
void *TestSendThread (void *vargp){
while(1){
	scanf("%c",&mode);
	if(mode=='n')
	{
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 12;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_Check, vrui_SHAREMESS_Send2GatewayLength);
		printf("Check node\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
	}
	else if(mode=='s')
	{
		printf ("Select node:\n");
//		uint8_t unicast1,unicast2;
		scanf("%x %x",&unicast1,&unicast2);
		//printf ("%d",unicast1);
		OUTMESSAGE_SelectNodeSend1[8]=unicast1;
		OUTMESSAGE_SelectNodeSend1[9]=unicast2;
		OUTMESSAGE_SelectNodeSend1[12]=unicast1;
		OUTMESSAGE_SelectNodeSend1[13]=unicast2;
		OUTMESSAGE_SelectNodeSend2[8]=unicast1;
		OUTMESSAGE_SelectNodeSend2[9]=unicast2;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 16;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_SelectNodeSend1, vrui_SHAREMESS_Send2GatewayLength);
		//printf("Select node 1\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(1);
		if(flag_selectnode1==1)
		{
			flag_selectnode1=0;
			pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
			vrb_SHAREMESS_Send2GatewayAvailabe = true;
			vrui_SHAREMESS_Send2GatewayLength = 12;
			memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_SelectNodeSend2, vrui_SHAREMESS_Send2GatewayLength);
			//printf("Select node 2\n");
			pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
			sleep(1);
		}
		if(flag_selectnode2==1)
		{
			flag_selectnode2=0;
			printf("Selected:%x %x\n",OUTMESSAGE_SelectNodeSend1[8],OUTMESSAGE_SelectNodeSend1[9]);
		}
	}
	else if(mode=='a'){
		OUTMESSAGE_Addgroup[8]=unicast1;
		OUTMESSAGE_Addgroup[9]=unicast2;
		OUTMESSAGE_Addgroup[12]=unicast1;
		OUTMESSAGE_Addgroup[13]=unicast2;
		printf ("Add into group: ");
		scanf("%x",&groupadd);
        OUTMESSAGE_Addgroup[14]=groupadd;
        pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 18;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_Addgroup, vrui_SHAREMESS_Send2GatewayLength);
		printf("Add %x %x into group %x\n",unicast1,unicast2,groupadd);
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(1);
	}
	else if(mode=='d')
	{
		OUTMESSAGE_Delgroup[8]=unicast1;
		OUTMESSAGE_Delgroup[9]=unicast2;
		OUTMESSAGE_Delgroup[12]=unicast1;
		OUTMESSAGE_Delgroup[13]=unicast2;
		printf ("Get out group: ");
		scanf("%x",&groupdel);
		OUTMESSAGE_Delgroup[14]=groupdel;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 18;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_Delgroup, vrui_SHAREMESS_Send2GatewayLength);
		printf("Delete %x %x get out group %x\n",unicast1,unicast2,groupdel);
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(1);
	}
	else if (mode=='c')
	{

		printf ("Control group: ");scanf("%x",&groupcontrol);
		OUTMESSAGE_ControlGroup[8]= groupcontrol;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 14;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_ControlGroup, vrui_SHAREMESS_Send2GatewayLength);
		printf("OFF\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(2);
		OUTMESSAGE_ControlGroup[12]=1;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 14;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_ControlGroup, vrui_SHAREMESS_Send2GatewayLength);
		printf("ON\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(2);
		OUTMESSAGE_ControlGroup[12]=0;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 14;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_ControlGroup, vrui_SHAREMESS_Send2GatewayLength);
		printf("OFF\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(2);
		OUTMESSAGE_ControlGroup[12]=1;
		pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
		vrb_SHAREMESS_Send2GatewayAvailabe = true;
		vrui_SHAREMESS_Send2GatewayLength = 14;
		memcpy(&vrsc_SHAREMESS_Send2GatewayMessage, &OUTMESSAGE_ControlGroup, vrui_SHAREMESS_Send2GatewayLength);
		printf("ON\n");
		pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
		sleep(2);
	}
}
	return NULL;
}



/*
 * Ham xu ly chinh. Thuc thi:
 * - Goi cac tac vu tuong ung cua chuong trinh
 * PENDING: Tiep tuc bo sung them cac Task khac
 */
int main(void) {
	puts("-----Start system123!-----");

	pthread_create(&vrts_System_ThreadGWIF, NULL, GWINF_Thread, NULL);
	pthread_create(&vrts_System_TestSend, NULL, TestSendThread, NULL);
	pthread_join(vrts_System_ThreadGWIF, NULL);
	pthread_join(vrts_System_TestSend, NULL);
	while(1){
	}
	return EXIT_SUCCESS;
}


