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
#include "smbus.h"
#include <stdio.h>


// CRC-8
void generate_crc(uint8_t* crc, uint8_t current_byte) {
    for (int i = 0; i < 8; i++) {
        bool apply = (current_byte >> (7 - i) & 1) ^ (*crc >> 7 & 1);
        *crc <<= 1;
        if (apply) *crc ^= 0x07;
    }
}

int generate_smbus_crc(uint8_t address, uint8_t cmd, uint8_t* reply, uint8_t length, bool is_block, bool is_read) {
    uint8_t crc = 0;

    generate_crc(&crc, (address << 1) + !is_read);  // address + write bit
    generate_crc(&crc, cmd);                        // command

    // reading causes direction flip, so the address occurs again
    if (is_read)
        generate_crc(&crc, (address << 1) + is_read);   // address + read bit

    // blocks prepend length to reply
    if (is_block)
        generate_crc(&crc, length);

    for (int i = 0; i < length; i++) {
        generate_crc(&crc, reply[i]);
    }

    return crc;
}

bool validate_smbus_crc(uint8_t address, uint8_t cmd, uint8_t* reply, uint8_t length, uint8_t recieved_crc, bool is_block, bool is_read) {
    int crc_gen = generate_smbus_crc(address, cmd, reply, length, is_block, is_read);
    if (recieved_crc == crc_gen) return true;

    printf("CRC invalid! recieved 0x%02x != calculated 0x%02x\n", recieved_crc, crc_gen);
    return false;
}


void i2c_stop_blocking(i2c_dev_t* device) {
    i2c_hw_t* hw = i2c_get_hw(device->i2c);

    i2c_get_hw(device->i2c)->enable = 0;
    i2c_get_hw(device->i2c)->tar = device->address;
    i2c_get_hw(device->i2c)->enable = 1;

    while (!i2c_get_write_available(device->i2c)) {
        tight_loop_contents();
    }

    hw->data_cmd = I2C_IC_DATA_CMD_STOP_LSB;

    bool abort;
    do abort = hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_ABRT_BITS;
    while (!i2c_get_read_available(device->i2c) && !abort);

    if (abort) hw->clr_tx_abrt;
    else i2c_get_hw(device->i2c)->data_cmd;

    device->i2c->restart_on_next = false;
}


/**
 * reads result from an smbus command with crc checking.
 * returns a negative value on error, or the number of bytes in the result.
 * if return value is non-negative it always equals the length.
 */
int smbus_read(i2c_dev_t* device, uint8_t cmd, uint8_t* result, size_t length) {
    printf("reading %d bytes from command 0x%02x\n", length, cmd);
    int ret;
    uint8_t crc;

    // write command code
    ret = i2c_write_blocking(device->i2c, device->address, &cmd, 1, true);
    if (ret < 0) {
        printf("failed, write returned %d\n", ret);
        return SMBUS_ERROR_DEVICE;
    }

    // read result
    ret = i2c_read_burst_blocking(device->i2c, device->address, result, length);
    if (ret < 0) {
        printf("failed, read returned %d\n", ret);
        return SMBUS_ERROR_DEVICE;
    }

    // read crc
    ret = i2c_read_blocking(device->i2c, device->address, &crc, 1, false);
    if (ret < 0) {
        printf("failed, crc read returned %d\n", ret);
        return SMBUS_ERROR_DEVICE;
    }
    
    if (!validate_smbus_crc(device->address, cmd, result, length, crc, false, true))
        return SMBUS_ERROR_CRC;

    return length;
}

/**
 * reads result block from an smbus command with crc checking.
 * returns a negative value on error, or the block length.
 */
int smbus_read_block(i2c_dev_t* device, uint8_t cmd, uint8_t* result, size_t max_length) {
    printf("reading block from command 0x%02x (max %d bytes)\n", cmd, max_length);
    int ret;
    uint8_t block_length, crc;

    // write command code
    ret = i2c_write_blocking(device->i2c, device->address, &cmd, 1, true);
    if (ret < 0) {
        printf("failed, write returned %d\n", ret);
        return SMBUS_ERROR_DEVICE;
    }

    // read block length
    ret = i2c_read_burst_blocking(device->i2c, device->address, &block_length, 1);
    if (ret < 0) {
        printf("failed, block size read returned %d\n", ret);
        return SMBUS_ERROR_DEVICE;
    }

    if (block_length > max_length) {
        // don't truncate, crc can't be verified
        printf("block is longer than max_length! block_length = %d, max_length = %d\n", block_length, max_length);
        i2c_stop_blocking(device);
        return SMBUS_ERROR_GENERIC;
    }

    // read block
    ret = i2c_read_burst_blocking(device->i2c, device->address, result, block_length);
    if (ret < 0) {
        printf("failed, block read returned %d\n", ret);
        return SMBUS_ERROR_DEVICE;
    }

    // read crc
    ret = i2c_read_blocking(device->i2c, device->address, &crc, 1, false);
    if (ret < 0) {
        printf("failed, crc read returned %d\n", ret);
        return SMBUS_ERROR_DEVICE;
    }

    if (!validate_smbus_crc(device->address, cmd, result, block_length, crc, true, true))
        return SMBUS_ERROR_CRC;

    return block_length;
}


/**
 * reads a uint16 result from an smbus command
 * returns a negative value on error, or the number of bytes in the result (always 2).
 */
int smbus_read_uint16(i2c_dev_t* device, uint8_t cmd, uint16_t* result) {
    return smbus_read(device, cmd, (uint8_t*) result, 2);
}

/**
 * reads a string result from an smbus command
 * returns a negative value on error, or the length of the string.
 * if the string contains a null byte, that null byte will be considered the end of the string.
 */
int smbus_read_text(i2c_dev_t* device, uint8_t cmd, char* result, size_t max_length) {
    int ret;
    
    ret = smbus_read_block(device, cmd, result, max_length);
    if (ret < 0) return ret;

    // sometimes text is null-terminated and the block length goes longer than the actual text
    for (int i = 0; i < ret; i++) {
        if (result[i] == 0x00) return i;
    }

    return ret;
}
