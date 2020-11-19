#include "../GatewayManager/ButtonManager.h"

bool flag_button1 = false;
bool flag_button2 = false;
bool flag_button3 = false;
bool flag_button4 = false;
bool flag_button5 = false;

bool IsButtonPress(unsigned int button)
{
	bool i;
	switch (button)
	{
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
	if(IsButtonPress(1)){
		flag_button1 = false;
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
