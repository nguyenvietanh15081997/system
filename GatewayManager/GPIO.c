///*
// * GPIO.c
// */
//#include "../GatewayManager/GPIO.h"
//
//uint16_t i=0;
//uint8_t flag_blink = 0;
//mraa_gpio_context led_ble, led_zingbee, led_service, led_internet;
//void Gpio_int(mraa_gpio_context gpio, uint8_t num_gpio, mraa_gpio_dir_t direction,uint8_t value)
//{
//	gpio = mraa_gpio_init(num_gpio);
//	mraa_gpio_dir(gpio,direction);
//	mraa_gpio_write(led_ble, value);
//}
//void blink(mraa_gpio_context gpio, uint16_t time1, uint16_t time2)
//{
//	while(flag_blink)
//	{
//		i++;
//		printf("%d\n",i);
//		if(i<=time1){
//			mraa_gpio_write(gpio, 0);
//		}
//		if(i> time1 && i>= time2){
//			mraa_gpio_write(gpio, 1);
//			if(i==time2) i=0;
//		}
//	}
//	mraa_gpio_write(gpio, 1);
//}
//void * GPIO_Thread(void * arvg)
//{
//	puts("hello");
////	Gpio_int(led_ble,LED_BLE,MRAA_GPIO_OUT);
////	Gpio_int(led_service,LED_SERVICE,MRAA_GPIO_OUT);
////	Gpio_int(led_internet,LED_INTERNET,MRAA_GPIO_OUT);
////	Gpio_int(led_zingbee,LED_ZINGBEE,MRAA_GPIO_OUT);
//	while(1)
//	{
//		blink(led_ble, 10000, 20000);
//	}
//	return NULL;
//}
#include "../GatewayManager/GPIO.h"



unsigned flagblink=0;

int gpio_export(unsigned gpio)
{
        int fd, len;
        char buf[11];
        fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd < 0) {
                perror("gpio/export");
                return fd;
        }
        len = snprintf(buf, sizeof(buf), "%d", gpio);
        write(fd, buf, len); //Ghi số hiệu (ID) pin muốn sử dụng vào file /sys/class/gpio/export khi đăng ký sử dụng
        close(fd);
        return 0;
}
 int gpio_unexport(unsigned gpio)
{
        int fd, len;
        char buf[11];
        fd = open("/sys/class/gpio/unexport", O_WRONLY);
        if (fd < 0) {
                perror("gpio/export");
                return fd;
        }
        len = snprintf(buf, sizeof(buf), "%d", gpio);
        write(fd, buf, len); //Ghi số hiệu (ID) pin muốn sử dụng vào file /sys/class/gpio/unexport khi giải phóng
        close(fd);
        return 0;
}
int gpio_dir(unsigned gpio, unsigned dir)
{
        int fd, len;
        char buf[60];
        len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);
        fd = open(buf, O_WRONLY);
        if (fd < 0) {
                perror("gpio/direction");
                return fd;
        }
        //Cấu hình pin là input/output bằng cách ghi giá trị (ASCII) in, out vào file /sys/class/gpio/gpio[ID]/diriection
        if (dir == GPIO_DIR_OUT)
                write(fd, "out", 4);
        else
                write(fd, "in", 3);
        close(fd);
        return 0;
}
int gpio_dir_out(unsigned gpio)
{
        return gpio_dir(gpio, GPIO_DIR_OUT); //trường hợp là output
}
int gpio_dir_in(unsigned gpio)
{
        return gpio_dir(gpio, GPIO_DIR_IN); //trường hợp là input
}
int gpio_value(unsigned gpio, unsigned value)
{
        int fd, len;
        char buf[60];
        len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
        fd = open(buf, O_WRONLY);
        if (fd < 0) {
                perror("gpio/value");
                return fd;
        }
        //Xuất giá trị 1, 0 bằng cách ghi ra file value tương ứng với pin đã cấu hình
        if (value)
                write(fd, "1", 2);
        else
                write(fd, "0", 2);
        close(fd);
        return 0;
}
void gpio_int(unsigned gpio, unsigned direction, unsigned value)
{
	gpio_export(gpio);
	if(!direction)
	{
		gpio_dir_out(gpio);
	}
	else gpio_dir_in(gpio);
	gpio_value(gpio, value);
}
void blink(unsigned gpio)
{
	//gpio_int(15, GPIO_DIR_OUT, 1);
	while(flagblink){
		gpio_value(gpio,0);
		sleep(1);
		gpio_value(gpio,1);
	    sleep(1);
	    puts("vietanh");
	}
}
void * GPIO_Thread(void * arvg)
{
	puts("HELLO");
	while(1)
	{
		//printf("%d\n",flagblink);
		blink(15);
	}
	return NULL;
}
