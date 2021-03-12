/*
 * OpCode.c
 */

#include "../GatewayManager/OpCode.h"

#define TYPE_LIGHT 		1
#define TYPE_SWITCH 	2
#define TYPE_SENSOR 	3
#define TYPE_REMOTE 	4
#define TYPE_SPEAKER 	5
#define TYPE_CAMERA 	6

#define ATTRUBUTE_LIGHT_DIM 			1
#define ATTRUBUTE_LIGHT_DIM_CCT 		2
#define ATTRUBUTE_LIGHT_RGB 			3
#define ATTRUBUTE_LIGHT_RGB_CCT 		4
#define ATTRUBUTE_LIGHT_RGB_CCT_SPEAKER 5

#define ATTRUBUTE_SWITCH_ONOFF (1)
#define ATTRUBUTE_SWITCH_TOUCH (2)
#define ATTRUBUTE_SWITCH_SCENE (3)

#define ATTRUBUTE_SENSOR_LIGHT 		(1)
#define ATTRUBUTE_SENSOR_LIGHT_PIR 	(2)
#define ATTRUBUTE_SENSOR_FIRE 		(3)
#define ATTRUBUTE_SENSOR_WATER 		(4)
#define ATTRUBUTE_SENSOR_GAS 		(5)
#define ATTRUBUTE_SENSOR_PROXIMITY 	(6)

#define ATTRUBUTE_REMOTE_INFRARED (1)

#define APPLI_DL_SMT 		(1)
#define APPLI_DL_COB 		(2)
#define APPLI_DL_GOCHEP 	(3)
#define APPLI_DL_AT19 		(4)
#define APPLI_PN_CIRCLE 	(5)
#define APPLI_PN_SQUARES 	(6)
#define APPLI_OPTRAN 		(7)
#define APPLI_OPTUONG 		(8)
#define APPLI_CHIEUTRANH 	(9)
#define APPLI_TRACKLIGHT 	(10)
#define APPLI_THATRAN 		(11)
#define APPLI_DENGUONG 		(12)
#define APPLI_LEDDAY_CCT 	(13)
#define APPLI_M16 			(14)
#define APPLI_DENBAN 		(15)
#define APPLI_FLOODLIGHT 	(16)

#define APPLI_BULB 		(1)
#define APPLI_LEDDAYRGB (2)

#define APPLI_OPTRANLOA (1)

#define APPLI_SWITCH_ONOFF (1)

#define APPLI_SWITCH_1 		(1)
#define APPLI_SWITCH_2 		(2)
#define APPLI_SWITCH_3 		(3)
#define APPLI_SWITCH_4 		(4)
#define APPLI_SWITCH_POWER 	(5)

#define APPLI_DC 		(1)
#define APPLI_AC 		(2)
#define APPLI_BIGTOUCH 	(3)

#define APPLI_SENSOR_GATE (4)

#define ID_DL_SMT 		(12001)
#define ID_DL_COB 		(12002)
#define ID_DL_GOCHEP 	(12003)
#define ID_DL_AT19 		(12004)
#define ID_PN_CIRCLE 	(12005)
#define ID_PN_SQUARES 	(12006)
#define ID_OPTRAN 		(12007)
#define ID_OPTUONG 		(12008)
#define ID_CHIEUTRANH 	(12009)
#define ID_TRACKLIGHT 	(12010)
#define ID_THATRAN 		(12011)
#define ID_DENGUONG 	(12012)
#define ID_LEDDAY_CCT 	(12013)
#define ID_M16 			(12014)
#define ID_DENBAN 		(12015)
#define ID_FLOODLIGHT 	(12016)

#define ID_BULB 		(14002)
#define ID_LEDDAYRGB    (14001)

#define ID_OPTRANLOA 	(15001)

#define ID_SWITCH_ONOFF (21001)

#define ID_CT01		(22001)
#define ID_CT04		(22004)

#define ID_SCM2_DC		(23001)
#define ID_SC03_AC		(23002)

#define ID_LS			(31001)
#define ID_LS_PIR		(32001)
// for mqtt control process
bool MODE_PROVISION = false;
bool check_add_or_del_group = false ;
bool check_add_or_del_scene = false ;


uint16_t TypeConvertID(uint8_t type, uint8_t attrubute, uint8_t application){
	uint16_t id = 65535;
	if((type != 255) && (attrubute != 255) && (application != 255)){
		id = (type*10000 + attrubute*1000 + application);
	}
	return id;
}
