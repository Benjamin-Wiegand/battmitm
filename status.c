/**
    MIT License

    Copyright (c) 2025 Benjamin Wiegand

    Permission is hereby granted, free of charge, to any person obtaining a copy 
    of this software and associated documentation files (the "Software"), to deal 
    in the Software without restriction, including without limitation the rights 
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
    copies of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.
 */
#include "status.h"
#include "config.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#define PWM_FULL 65535
#define PWM_HALF PWM_FULL / 2
#define PWM_OFF 0

#define STATUS_ITEMS 1
#define STATUS_ITEM_MITM_INDEX 0


bool status_item_states[STATUS_ITEMS];


void init_status() {
    pwm_config config = pwm_get_default_config();

    gpio_set_function(STATUS_LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(STATUS_LED_PIN);
    pwm_init(slice_num, &config, true);

    // "I'm alive"
    pwm_set_gpio_level(STATUS_LED_PIN, PWM_HALF);
    sleep_ms(100);
    pwm_set_gpio_level(STATUS_LED_PIN, PWM_OFF);
    sleep_ms(100);
    pwm_set_gpio_level(STATUS_LED_PIN, PWM_HALF);
    sleep_ms(100);
    pwm_set_gpio_level(STATUS_LED_PIN, PWM_OFF);
    
}

void status_led_update() {
    uint16_t brightness = 0;
    uint16_t brightness_steps = STATUS_LED_MAX_BRIGHTNESS / STATUS_ITEMS;

    for (int i = 0; i < STATUS_ITEMS; i++) {
        brightness += brightness_steps * status_item_states[i];
    }

    pwm_set_gpio_level(STATUS_LED_PIN, brightness);
}


void status_mitm(bool activity) {
    status_item_states[STATUS_ITEM_MITM_INDEX] = activity;
    status_led_update();
}
