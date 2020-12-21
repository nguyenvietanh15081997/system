#include "../GatewayManager/Ds1307.h"
#include "../GatewayManager/Light.h"
#include "../GatewayManager/JsonProcess.h"

uint8_t dataDecTimeDs1307[7];
uint8_t dataBcdTimeDs1307[7];
uint8_t dataTimeInternet[7];

#define I2C_BUS     0
#define DS1307_ADDR 0x68

mraa_i2c_context i2c;


uint8_t  BCD2DEC(uint8_t dataBCD)
{
	return ((dataBCD>>4)*10 + (dataBCD&0x0F));
}

uint8_t DEC2BCD(uint8_t dataDEC)
{
	return ((dataDEC/10)<<4 |(dataDEC%10));
}

void Ds1307_Init()
{

	i2c = mraa_i2c_init(I2C_BUS);
	mraa_i2c_frequency(i2c, 0);
	mraa_i2c_address(i2c, DS1307_ADDR);
}

void TimeForDs1307()
{
	mraa_i2c_read_bytes_data(i2c, 0x00, dataBcdTimeDs1307, 7);
	int i;
	for(i=0;i<7;i++){
		dataDecTimeDs1307[i]= BCD2DEC(dataBcdTimeDs1307[i]);
	}

	for(i=0;i<7;i++){
		printf("%d-",dataDecTimeDs1307[i]);
	}
}

void TimeForInternet()
{
	FILE *fp;
	  char path[1035];
	  fp = popen("date", "r");
	  if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	  }
	  while (fgets(path, sizeof(path), fp) != NULL) {
		  break;
	  }
	  pclose(fp);
	  if(path[0]=='M' && path[1]=='o' && path[2]=='n'){
		dataTimeInternet[3]=2;
	  }
	  if(path[0]=='T' && path[1]=='u' && path[2]=='e'){
		dataTimeInternet[3]=3;
	  }
	  if(path[0]=='W' && path[1]=='e' && path[2]=='d'){
		dataTimeInternet[3]=4;
	  }
	  if(path[0]=='T' && path[1]=='h' && path[2]=='u'){
		dataTimeInternet[3]=5;
	  }
	  if(path[0]=='F' && path[1]=='r' && path[2]=='i'){
		dataTimeInternet[3]=6;
	  }
	  if(path[0]=='S' && path[1]=='a' && path[2]=='t'){
		dataTimeInternet[3]=7;
	  }
	  if(path[0]=='S' && path[1]=='u' && path[2]=='n'){
		dataTimeInternet[3]=1;
	  }

	  if(path[4]=='J' && path[5]=='a' && path[6]=='n'){
		  dataTimeInternet[5]=1;
	  }
	  if(path[4]=='F' && path[5]=='e' && path[6]=='b'){
		  dataTimeInternet[5]=2;
	  }
	  if(path[4]=='M' && path[5]=='a' && path[6]=='r'){
		  dataTimeInternet[5]=3;
	  }
	  if(path[4]=='A' && path[5]=='p' && path[6]=='r'){
		  dataTimeInternet[5]=4;
	  }
	  if(path[4]=='M' && path[5]=='a' && path[6]=='y'){
		  dataTimeInternet[5]=5;
	  }
	  if(path[4]=='J' && path[5]=='u' && path[6]=='n'){
		  dataTimeInternet[5]=6;
	  }
	  if(path[4]=='J' && path[5]=='u' && path[6]=='l'){
		  dataTimeInternet[5]=7;
	  }
	  if(path[4]=='A' && path[5]=='u' && path[6]=='g'){
		  dataTimeInternet[5]=8;
	  }
	  if(path[4]=='S' && path[5]=='e' && path[6]=='p'){
		  dataTimeInternet[5]=9;
	  }
	  if(path[4]=='O' && path[5]=='c' && path[6]=='t'){
		  dataTimeInternet[5]=10;
	  }
	  if(path[4]=='N' && path[5]=='o' && path[6]=='v'){
		  dataTimeInternet[5]=11;
	  }
	  if(path[4]=='D' && path[5]=='e' && path[6]=='c'){
		  dataTimeInternet[5]=12;
	  }
	  dataTimeInternet[0]= (path[11]-48) * 10 + (path[12]-48);
	  dataTimeInternet[1]= (path[14]-48) * 10 + (path[15]-48);
	  dataTimeInternet[2]= (path[17]-48) * 10 + (path[18]-48);
	  dataTimeInternet[4]= (path[8]-48) * 10 +  (path[9]-48);
	  dataTimeInternet[6]= (path[26]-48) * 10 + (path[27]-48);
	  int n;
	  for(n=0;n<7;n++){
		  printf ("%d-",dataTimeInternet[n]);
	  }
	  printf("\n");
}
void SetTimeForDs1307(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t days, uint8_t dates, uint8_t months, uint8_t years)
{
	mraa_i2c_write_byte_data(i2c, DEC2BCD(seconds), 0x00);
	mraa_i2c_write_byte_data(i2c, DEC2BCD(minutes), 0x01);
	mraa_i2c_write_byte_data(i2c, DEC2BCD(hours), 0x02);
	mraa_i2c_write_byte_data(i2c, DEC2BCD(days), 0x03);
	mraa_i2c_write_byte_data(i2c, DEC2BCD(dates), 0x04);
	mraa_i2c_write_byte_data(i2c, DEC2BCD(months), 0x05);
	mraa_i2c_write_byte_data(i2c, DEC2BCD(years), 0x06);
}
void * Time_Thread(void *argv)
{
	//Ds1307_Init();
	while(1)
	{
		TimeForInternet();
//		if(dataTimeInternet[0]==vrts_Json_String.hours && dataTimeInternet[1]==15){
//			//JsonControl(key);
//		}
	}
	return NULL;
}
