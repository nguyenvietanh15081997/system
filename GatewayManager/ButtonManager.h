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

// opcode of button
#define BUTTON_GATEWAY_CMD          0xA082
#define BUTTON_GATEWAY_RSP          0xA182

//flag
extern bool flag_button1;
extern bool flag_button2;
extern bool flag_button3;
extern bool flag_button4;
extern bool flag_button5;

bool IsButtonPress(unsigned int button);
void ProcessButton();

#ifdef __cplusplus
}
#endif



#endif
