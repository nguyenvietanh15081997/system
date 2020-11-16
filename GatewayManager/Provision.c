
#include "../GatewayManager/Provision.h"
pthread_t tmp;

void controlmessage(uint16_t lengthmessage,uint8_t *Message)
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
	while(0){
		if((flag_done == true) || (Timeout_CheckDataBuffer == 32000))
		{
			flag_done = false;
			Timeout_CheckDataBuffer=0;
			sleep(1);
			controlmessage(3, OUTMESSAGE_ScanStart);
			printf ("scan\n");
			flag_check_select_mac= true;
		}

		if((flag_selectmac==true) && (flag_mac==true) )
		{
			flag_selectmac=false;
			flag_mac=false;
			controlmessage(9, OUTMESSAGE_MACSelect);
			printf("selectmac\n");
			sleep(2);

			controlmessage(3, OUTMESSAGE_GetPro);
			printf ("getpro\n");
			sleep(1);
		}
		if((flag_getpro_info == true) && (flag_getpro_element == true))
		{
			flag_getpro_info = false;
			flag_getpro_element = false;
			controlmessage(28, OUTMESSAGE_Provision);
			printf ("provision\n");
		}

		if(flag_provision == true)
		{
			flag_provision=false;
			sleep(1);
			controlmessage(22, OUTMESSAGE_BindingALl);
			printf ("binding all\n");
		}
	}
	return NULL;
}
