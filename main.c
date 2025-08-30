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
#include "mitm.h"
#include "smbus.h"
#include "status.h"
#include "config.h"
#include <stdio.h>
#include "pico/stdlib.h"


void init_i2c() {
    // init batt i2c
    gpio_set_function(BATT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BATT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    i2c_init(BATT_I2C, BATT_I2C_BAUD);
    if (BATT_I2C_PULL_UP) {
        gpio_pull_up(BATT_I2C_SDA_PIN);
        gpio_pull_up(BATT_I2C_SCL_PIN);
    }

    // init laptop i2c
    gpio_set_function(LAPTOP_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(LAPTOP_I2C_SCL_PIN, GPIO_FUNC_I2C);
    i2c_init(LAPTOP_I2C, LAPTOP_I2C_BAUD);
    if (LAPTOP_I2C_PULL_UP) {
        gpio_pull_up(LAPTOP_I2C_SDA_PIN);
        gpio_pull_up(LAPTOP_I2C_SCL_PIN);
    }

    i2c_set_slave_mode(LAPTOP_I2C, true, LAPTOP_I2C_ADDR);
}


int main() {
    stdio_init_all();

    sleep_ms(2000);

    printf("\n\n\n======= init =======\n\n\n");

    i2c_dev_t* bms = get_bms_dev();
    i2c_dev_t* laptop = get_laptop_dev();
    
    init_status();
    init_i2c();
    init_mitm();

    while (true) {
        mitm_loop();
    }
}
