#include "../GatewayManager/ButtonManager.h"
#include "../GatewayManager/Provision.h"

uint8_t ONMES1[14]  = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x01, 0x00};
uint8_t OFFMES1[14] = {0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0xff, 0xff, 0x82, 0x02, 0x00, 0x00};

bool flag_button0 = false;
bool flag_button1 = false;
bool flag_button2 = false;
bool flag_button3 = false;
bool flag_button4 = false;
bool flag_button5 = false;

bool IsButtonPress(unsigned int button)
{
	bool i= false;
	switch (button)
	{
	case 0:
		i = flag_button0;
		break;
	case 1:
		i = flag_button1;
		break;
	case 2:
		i = flag_button2;
		break;
	case 3:
		i = flag_button3;
		break;
	case 4:
		i = flag_button4;
		break;
	case 5:
		i = flag_button5;
		break;
	}
	return i;
}
void ProcessButton()
{
	if(flag_button0){
		flag_button0= false;
		ControlMessage(14, OFFMES1);
		puts("OFF");
		sleep(2);
	}
	if(flag_button1){
		flag_button1 = false;
		ControlMessage(14, ONMES1);
		puts("ON");
		sleep(2);
	}
	if(IsButtonPress(2)){
		flag_button2 = false;
	}
	if(IsButtonPress(3)){
		flag_button1 = false;
	}
	if(IsButtonPress(4)){
		flag_button2 = false;
	}
	if(IsButtonPress(5)){
		flag_button2 = false;
	}

}
