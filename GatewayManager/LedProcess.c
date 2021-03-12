#include "LedProcess.h"


mraa_gpio_context *gpio;
bool flag_blink = false;
bool flag_close_gpio = false;
pthread_t tmp1;


void gpio_pin_write_on(mraa_gpio_context gpio){
    mraa_gpio_write(gpio, 0);
}

void gpio_pin_write_off(mraa_gpio_context gpio){
    mraa_gpio_write(gpio, 1);
}

void led_pin_init(){
	int i;
    uint8_t pins_map[] = LED_PINS_MAP;
    uint8_t pins_index[] = LED_PINS_INDEX_MAP;
    uint8_t num_of_led_pin = sizeof(pins_map)/sizeof(uint8_t);
    gpio = (mraa_gpio_context *)malloc(num_of_led_pin * sizeof(mraa_gpio_context));
    for(i = 0; i < num_of_led_pin; i++){
        gpio[pins_index[i]] = mraa_gpio_init(pins_map[i]);
        mraa_gpio_dir(gpio[pins_index[i]], MRAA_GPIO_OUT);
    }
    mraa_gpio_write(gpio[LED_ZIGBEE_PIN_INDEX], 1);
    mraa_gpio_write(gpio[LED_BLE_PIN_INDEX], 1);
}
void led_pin_close(){
	uint8_t pins_map[] = LED_PINS_MAP;
	int i;
	for(i = 0; i < sizeof(pins_map)/sizeof(uint8_t); i++){
		mraa_gpio_close(gpio[i]);
	}
}
void led_pin_on(mraa_gpio_context led_pin){
    gpio_pin_write_on(led_pin);
    puts("on");
}

void led_pin_off(mraa_gpio_context led_pin){
    gpio_pin_write_off(led_pin);
    puts("off");
}

void led_pin_blink_once(mraa_gpio_context led_pin){
		led_pin_on(led_pin);
		sleep(1);
		led_pin_off(led_pin);
		sleep(1);
}

void* Led_Thread(void* arvg){
    uint8_t pins_map[] = LED_PINS_MAP;
    int i;
    led_pin_init();
    tmp1 = pthread_self();
    while(flag_blink){
    	led_pin_blink_once(gpio[LED_BLE_PIN_INDEX]);
    }
//	for(i = 0; i < sizeof(pins_map)/sizeof(uint8_t); i++){
//		mraa_gpio_close(gpio[i]);
//	}
    return NULL;
}
