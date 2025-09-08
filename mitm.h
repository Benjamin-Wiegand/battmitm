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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MITM_QUEUE_MAX_ELEMENTS 100

#define MITM_CMD_BUFFER_SIZE 64
#define MITM_REPLY_BUFFER_SIZE 64


// used to override the reply for any SBS read command
// cmd -> sbs command
// reply_buffer -> a buffer to place the new reply into. max size = MITM_REPLY_BUFFER_SIZE 
// return value -> negative value if an error has occured
typedef int (*cmd_reply_override)(uint8_t cmd, uint8_t* reply_buffer);

// reads the next "length" bytes from the battery into a buffer.
// returns the number of bytes read or a negative value if an error occured.
// for use inside command reply overrides.
int mitm_read_batt_reply(uint8_t* buffer, size_t length);

// validates the crc checksum at the end of the current command reply inside a buffer.
// crc_index is both the length of the reply (excluding crc) and the index where the crc is located in the buffer.
// set is_block to true if the reply is a block read.
// returns true if valid, or false if corrupted.
// for use inside command reply overrides.
bool mitm_validate_batt_reply(uint8_t* buffer, uint8_t crc_index, bool is_block);

// generates a valid crc checksum at the end of a command reply.
// crc_index is both the length of the reply (excluding crc) and the index where the crc will be placed in the buffer.
// set is_block to true if the reply is a block read.
// for use inside command reply overrides.
// returns a negative value if a sanity check fails (crc is inherrently invalid).
int mitm_generate_reply_crc(uint8_t* buffer, uint8_t crc_index, bool is_block);



void init_mitm();
void mitm_loop();


// like the smbus read functions but applies an override
int mitm_smbus_read_with_override(i2c_dev_t* device, uint8_t cmd, uint8_t* result, size_t length, bool is_block, cmd_reply_override override);
int mitm_smbus_read_text_with_override(i2c_dev_t* device, uint8_t cmd, char* result, size_t max_length, cmd_reply_override override);
