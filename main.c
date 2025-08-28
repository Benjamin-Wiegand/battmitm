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
#include <stdio.h>
#include "pico/stdlib.h"
#include "smbus.h"


// use on-board led
#define STATUS_LED_PIN 25


#define BATT_I2C i2c0
#define BATT_I2C_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
#define BATT_I2C_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN

#define BATT_I2C_ADDR 0x0b
#define BATT_I2C_BAUD 32000

#define BATT_CMD_VOLTAGE 0x09
#define BATT_CMD_SERIAL 0x1c
#define BATT_CMD_MANUFACTURER_ACCESS 0x00
#define BATT_CMD_VENDOR 0x20


void status_led(int state) {
    gpio_put(STATUS_LED_PIN, state);
}


int main() {
    stdio_init_all();

    sleep_ms(2000);

    printf("\n\n\n======= init =======\n\n\n");

    gpio_init(STATUS_LED_PIN);
    gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);

    i2c_init(BATT_I2C, BATT_I2C_BAUD);
    gpio_set_function(BATT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BATT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BATT_I2C_SDA_PIN);
    gpio_pull_up(BATT_I2C_SCL_PIN);


    i2c_dev_t bms = {i2c_default, BATT_I2C_ADDR};


    int len;
    uint16_t voltage, serial, mf_acc;
    char vendor[32];

    while (true) {
        sleep_ms(1000);

        status_led(1);    
        printf("reading\n");


        len = smbus_read_uint16(&bms, BATT_CMD_MANUFACTURER_ACCESS, &mf_acc);
        if (len > 0) printf("mf access = %04x\n", mf_acc);
        else printf("ERROR %d\n", len);

        len = smbus_read_uint16(&bms, BATT_CMD_VOLTAGE, &voltage);
        if (len > 0) printf("voltage = %d mV\n", voltage);
        else printf("ERROR %d\n", len);

        len = smbus_read_uint16(&bms, BATT_CMD_SERIAL, &serial);
        if (len > 0) printf("serial = %d\n", serial);
        else printf("ERROR %d\n", len);

        len = smbus_read_text(&bms, BATT_CMD_VENDOR, &vendor[0], 32);
        if (len > 0) printf("vendor[%d] = %.*s\n", len, len, vendor);
        else printf("ERROR %d\n", len);

        status_led(0);
        printf("done\n\n");

    }
}
