#ifndef __LED_PROCESS_H__
#define __LED_PROCESS_H__

#include "mraa.h"
#include <stdio.h>
#include <stdbool.h>

extern pthread_t tmp1 ;

#define LED_INTERNET_PIN            19
#define LED_SERVICE_PIN             18
#define LED_ZIGBEE_PIN              16
#define LED_BLE_PIN                 15

#define LED_INTERNET_PIN_INDEX      0
#define LED_SERVICE_PIN_INDEX       1
#define LED_ZIGBEE_PIN_INDEX        2
#define LED_BLE_PIN_INDEX           3

#define LED_PINS_MAP                 {LED_INTERNET_PIN, LED_SERVICE_PIN, LED_ZIGBEE_PIN, LED_BLE_PIN}
#define LED_PINS_INDEX_MAP           {LED_INTERNET_PIN_INDEX, LED_SERVICE_PIN_INDEX, LED_ZIGBEE_PIN_INDEX, LED_BLE_PIN_INDEX}

extern bool flag_blink;
extern bool flag_close_gpio;

extern mraa_gpio_context *gpio;

void gpio_pin_write_on(mraa_gpio_context gpio);

void gpio_pin_write_off(mraa_gpio_context gpio);

void led_pin_init();

void led_pin_close();

void led_pin_on(mraa_gpio_context led_pin);

void led_pin_off(mraa_gpio_context led_pin);

extern void* Led_Thread(void* arvg);


#endif
