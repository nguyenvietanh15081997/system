#ifndef GATEWAYMANAGER_BUTTONMANAGER_H_
#define GATEWAYMANAGER_BUTTONMANAGER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef struct remotersp
{
	uint8_t   typeDev[2];
	uint8_t   buttonID;
	uint8_t   modeID;
	uint8_t   senceID[2];
	uint8_t   futureID[2];
}remotersp;
extern remotersp * vrts_Remote_Rsp;

#define ButtonID0           0x00
#define ButtonID1           0x01
#define ButtonID2           0x02
#define ButtonID3           0x03
#define ButtonID4           0x04
#define ButtonID5           0x05

#define ModeIDClick         0x01
#define ModeIDDouble        0x03
#define ModeIDHold          0x05

#define OPCODEREMOTE_CMD     0xA082
#define OPCODEREMOTE_RSP     0xA182


bool IsRemoteSetup(remotersp * rsp,unsigned char parButtonId,unsigned char parModeId,unsigned char parSenceId1,unsigned char parSenceId2);

#ifdef __cplusplus
}
#endif



#endif
