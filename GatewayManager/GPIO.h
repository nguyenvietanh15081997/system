
/*
 * GPIO.h 
 */
/*
#ifndef GATEWAYMANAGER_GPIO_H_
#define GATEWAYMANAGER_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mraa/gpio.h"

#define LED_BLE 15
#define LED_ZINGBEE 16
#define LED_SERVICE 18
#define LED_INTERNET 19
#define BUTTON_RESET_HC 2
extern uint8_t flag_blink;
extern mraa_gpio_context led_ble, led_zingbee, led_service, led_internet;

void Gpio_int(mraa_gpio_context gpio, uint8_t num_gpio, mraa_gpio_dir_t direction,uint8_t value);
void blink(mraa_gpio_context gpio, uint16_t time1, uint16_t time2);
void * GPIO_Thread(void * arvg);
#ifdef __cplusplus
}
#endif

#endif
*/
#ifndef GATEWAYMANAGER_GPIO_H_
#define GATEWAYMANAGER_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define GPIO_DIR_IN     0
#define GPIO_DIR_OUT    1
extern unsigned flagblink;

int gpio_export(unsigned gpio);  //Hàm export pin ra user space

int gpio_unexport(unsigned gpio); //Hàm giải phóng pin khi không còn sử dụng

int gpio_dir_out(unsigned gpio); //Cấu hình pin là output

int gpio_dir_in(unsigned gpio); //Cấu hình pin là input

int gpio_value(unsigned gpio, unsigned value); //Đọc/ghi giá trị của pin gpio

void gpio_int(unsigned gpio, unsigned direction, unsigned value);

void blink(unsigned gpio);

void * GPIO_Thread(void * arvg);

#ifdef __cplusplus
}
#endif

#endif