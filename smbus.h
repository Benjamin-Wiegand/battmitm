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
 #include "hardware/i2c.h"

#define SMBUS_ERROR_DEVICE -1
#define SMBUS_ERROR_GENERIC -2
#define SMBUS_ERROR_CRC -3


struct i2c_dev {
    i2c_inst_t* i2c;
    uint8_t address;
};

typedef struct i2c_dev i2c_dev_t;

int smbus_read(i2c_dev_t* device, uint8_t cmd, uint8_t* result, size_t length);
int smbus_read_block(i2c_dev_t* device, uint8_t cmd, uint8_t* result, size_t max_length);

int smbus_read_uint16(i2c_dev_t* device, uint8_t cmd, uint16_t* result);
int smbus_read_text(i2c_dev_t* device, uint8_t cmd, char* result, size_t max_length);


