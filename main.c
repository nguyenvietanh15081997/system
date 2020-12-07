/*
 ============================================================================
 Name        : GatewayManager.c
 Author      : haha
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "GatewayManager/GateInterface.h"
#include "GatewayManager/Provision.h"
#include "GatewayManager/MQTT.h"
#include "GatewayManager/SensorLight.h"

pthread_t vrts_System_ThreadGWIF;
pthread_t vrts_System_TestSend;
pthread_t vrts_System_MQTT;



/*z
 * Ham xu ly chinh. Thuc thi:
 * - Goi cac tac vu tuong ung cua chuong trinh
 * PENDING: Tiep tuc bo sung them cac Task khac
 */
int main(void) {

	puts("--------------Start system123-----------------");
	flag_mac=true;
	flag_done=true;
	pthread_create(&vrts_System_TestSend,NULL, ProvisionThread, NULL);
    pthread_create(&vrts_System_ThreadGWIF, NULL, GWINF_Thread, NULL);
    pthread_create(&vrts_System_MQTT,   NULL,   MQTT_Thread,    NULL);
	pthread_join(vrts_System_ThreadGWIF, NULL);
	pthread_join(vrts_System_TestSend, NULL);
    pthread_join(vrts_System_MQTT,NULL);
	while(1){
	}
	return EXIT_SUCCESS;
}
