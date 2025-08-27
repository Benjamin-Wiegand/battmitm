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


