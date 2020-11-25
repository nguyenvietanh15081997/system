
#include "../GatewayManager/Provision.h"

pthread_t tmp;

// timeout check data in buffer
unsigned int Timeout_CheckDataBuffer = 0;

bool flag_selectmac     	= false;
bool flag_getpro_info   	= false;
bool flag_getpro_element	= false;
bool flag_provision     	= false;
bool flag_mac           	= true;
bool flag_check_select_mac  = false;
bool flag_done          	= true;

void ControlMessage(uint16_t lengthmessage,uint8_t *Message)
{
	pthread_mutex_lock(&vrpth_SHAREMESS_Send2GatewayLock);
	vrb_SHAREMESS_Send2GatewayAvailabe = true;
	vrui_SHAREMESS_Send2GatewayLength = lengthmessage;
	memcpy(vrsc_SHAREMESS_Send2GatewayMessage, Message, vrui_SHAREMESS_Send2GatewayLength);
	pthread_mutex_unlock(&vrpth_SHAREMESS_Send2GatewayLock);
}


void *ProvisionThread (void *argv )
{
	tmp = pthread_self();
	while(MODE_PROVISION){
		if((flag_done == true) || (Timeout_CheckDataBuffer == 32000))
		{
			flag_done = false;
			Timeout_CheckDataBuffer=0;
			sleep(1);
			ControlMessage(3, OUTMESSAGE_ScanStart);
			printf ("scan\n");
			flag_check_select_mac= true;
		}

		if((flag_selectmac==true) && (flag_mac==true) )
		{
			flag_selectmac=false;
			flag_mac=false;
			ControlMessage(9, OUTMESSAGE_MACSelect);
			printf("selectmac\n");
			sleep(2);

			ControlMessage(3, OUTMESSAGE_GetPro);
			printf ("getpro\n");
			sleep(1);
		}
		if((flag_getpro_info == true) && (flag_getpro_element == true))
		{
			flag_getpro_info = false;
			flag_getpro_element = false;
			ControlMessage(28, OUTMESSAGE_Provision);
			printf ("provision\n");
		}

		if(flag_provision == true)
		{
			flag_provision=false;
			sleep(1);
			ControlMessage(22, OUTMESSAGE_BindingALl);
			printf ("binding all\n");
		}
	}
/*
while(1){
	if(flagLux == 1){
		ControlMessage(14, ONMES);
		puts("on");
		sleep(2);
	}
	else if(flagLux == 2){
		ControlMessage(14, OFFMES);
		puts("off");
		sleep(2);
	}
}*/
	return NULL;
}
