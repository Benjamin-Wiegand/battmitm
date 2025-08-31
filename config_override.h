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
#include "battery.h"
#include <stdio.h>


/* ================================= DANGER!!!! READ THIS FIRST ================================= *
 * If you don't know what you're doing, do not touch this file. I cannot stress this enough.      *
 * Modifying reponses from the BMS can have disastrous consequences when done wrong.              *
 *                                                                                                *
 * I am not responsible for property damage or bolidly harm as a result of modifying this file.   *
 *                                                                                                *
 *                                    YOU HAVE BEEN WARNED.                                       *
 * ============================================================================================== */


/*
    this file lets you define overrides that modify communications between your laptop and your BMS.

    currently there is only one supported override type: read command reply override

    fortunately, this one override type encompasses 99% of useful modifications.


    read command reply overrides allow you to replace or modify the reply of any read command 
    issued by the laptop (for the battery). they are defined in the form of functions as to not 
    limit flexibility.

    in a read command reply override function you can:
     - read bytes from the battery (+ verify crc) to get the real response
     - write a custom reply to the reply_buffer that the laptop will read from
     - generate a crc to be inserted at the end of your custom reply
     - return a negative value to trash the response if an error occurs
     - anything else that you can normally do in C on a microcontroller

    in a read command reply override function you cannot:
     - know the full response length from the battery (i2c limitation)
     - know how many bytes the laptop will read from the reply_buffer (i2c limitation)
     - explicitly abort the transfer (i2c limitation)
     - tell the laptop not to check the crc (you just can't ok)

    the examples defined below in this file show how to successfully replace/modify responses from
    the bms.
    
    for more information on SBS commands and their nuances, consult the spec: https://sbs-forum.org/specs/

    once you have your function defined, remember to add it to the switch statement at the bottom 
    of this file to enable it!
*/


// example read command reply override that returns a custom serial number
int override_example_replace_serial_number(uint8_t cmd, uint8_t* reply_buffer) {
    if (cmd != BATT_CMD_SERIAL_NUMBER) return -1;

    // you can use any 2-byte serial number here
    uint16_t serial_number = 6969;

    // low byte is sent first
    reply_buffer[0] = serial_number & 0x00FF;
    reply_buffer[1] = (serial_number & 0xFF00) >> 8;

    // generate a crc-8 checksum for the new reply
    if (mitm_generate_reply_crc(reply_buffer, 2, false) < 0) return -1;

    return 0;
}

// example read command reply override that returns a custom manufacturer name
int override_example_replace_manufacturer_name(uint8_t cmd, uint8_t* reply_buffer) {
    if (cmd != BATT_CMD_MANUFACTURER_NAME) return -1;

    // set whatever string you want here as long as it fits within the reply buffer (MITM_REPLY_BUFFER_SIZE - 2)
    char manufacturer_name[] = "BattMITM";
    uint8_t length = sizeof(manufacturer_name);

    // set block length (in this case the string length)
    reply_buffer[0] = length;
    
    // add block content
    for (int i = 0; i < length; i++) {
        reply_buffer[i+1] = manufacturer_name[i];
    }

    // generate a crc-8 checksum for the new reply
    // note: is_block is true since the manufacturer name uses a block read!
    if (mitm_generate_reply_crc(reply_buffer, length + 1, true) < 0) return -1;

    return 0;
}

// example read command reply override that replaces all 'A's in the manufacturer name with 'O's
int override_example_modify_manufacturer_name(uint8_t cmd, uint8_t* reply_buffer) {
    if (cmd != BATT_CMD_MANUFACTURER_NAME) return -1;
    
    int reply_index = 0;
    int ret;
    uint8_t block_length;

    // read the block length
    ret = mitm_read_batt_reply(reply_buffer, 1);
    if (ret < 0) return -1;
    reply_index += ret;
    block_length = reply_buffer[0];

    // ensure we won't overrun the buffer, considering 2 bytes for block length and crc
    if (block_length + 2 > MITM_REPLY_BUFFER_SIZE) {
        // this could be due to a corrupted reply from the battery
        printf("reply won't fit in buffer! reply_length = %d, buffer_size = %d\n", block_length + 2, MITM_REPLY_BUFFER_SIZE);
        return -1;
    }

    // read the block content + crc
    ret = mitm_read_batt_reply(reply_buffer + reply_index, block_length + 1);
    if (ret < 0) return -1;
    reply_index += ret;
    
    // validate the crc
    if (!mitm_validate_batt_reply(reply_buffer, reply_index - 1, true)) return -1;


    // modify block content (replace all 'A's with 'O's)
    for (int i = 1; i < reply_index - 1; i++) {
        if (reply_buffer[i] == 'A') {
            reply_buffer[i] = 'O';
        }
    }

    
    // generate a crc-8 checksum for the new reply
    if (mitm_generate_reply_crc(reply_buffer, reply_index - 1, true) < 0) return -1;

    return 0;
}



// map command codes to override functions here
// for a list of command code definitions see battery.h
cmd_reply_override get_read_command_reply_override(uint8_t cmd) {
    switch (cmd) {

        // examples:
        //case BATT_CMD_SERIAL_NUMBER:        return &override_example_replace_serial_number;
        //case BATT_CMD_MANUFACTURER_NAME:    return &override_example_replace_manufacturer_name;
        //case BATT_CMD_MANUFACTURER_NAME:    return &override_example_modify_manufacturer_name;

        default: return NULL;   // no override
    }
}





