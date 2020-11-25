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
//#define BUTTON_GATEWAY_CMD          0xA082
#define BUTTON_GATEWAY_RSP          0x52

#define Button0Press           0x00
#define Button1Press           0x01
#define Button2Press           0x02
#define Button3Press           0x03
#define Button4Press           0x04
#define Button5Press           0x05

//flag
extern bool flag_button0;
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
