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

// use on-board led
#define STATUS_LED_PIN 25

// i2c connected to battery (as master)
#define BATT_I2C i2c0
#define BATT_I2C_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN   // 4
#define BATT_I2C_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN   // 5
#define BATT_I2C_PULL_UP true           // disable this if you have an external resistor

#define BATT_I2C_ADDR 0x0b
#define BATT_I2C_BAUD 32000

// i2c connected to laptop (as slave)
#define LAPTOP_I2C i2c1
#define LAPTOP_I2C_SDA_PIN 2
#define LAPTOP_I2C_SCL_PIN 3
#define LAPTOP_I2C_PULL_UP false        // enable this if your laptop smbus is broken ig

#define LAPTOP_I2C_ADDR BATT_I2C_ADDR   // use the same address
#define LAPTOP_I2C_BAUD BATT_I2C_BAUD   // use the same baud (for now)


