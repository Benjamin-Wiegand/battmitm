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
#include "pico/i2c_slave.h"
#include "static_queue.h"
#include <stdio.h>


struct smbus_transfer {
    i2c_slave_event_t event;
    uint8_t data;
};

typedef struct smbus_transfer smbus_transfer_t;


#define MITM_QUEUE_ELEMENT_SIZE sizeof(smbus_transfer_t)
#define MITM_QUEUE_MAX_ELEMENTS 10

#define MITM_CMD_BUFFER_SIZE 64
#define MITM_REPLY_BUFFER_SIZE 64


static_queue_t* mitm_transfer_queue;
bool mitm_transfer_queue_overflow = false;

i2c_slave_event_t previous_event = I2C_SLAVE_FINISH;

uint8_t mitm_cmd_buffer[MITM_CMD_BUFFER_SIZE];
size_t mitm_cmd_buffer_index = 0;

uint8_t mitm_reply_buffer[MITM_REPLY_BUFFER_SIZE];
size_t mitm_reply_buffer_index = 0;


void mitm_laptop_irq_handler(i2c_inst_t* i2c, i2c_slave_event_t event) {
    smbus_transfer_t* transfer = static_queue_add(mitm_transfer_queue);
    if (transfer == NULL) return; //todo

    transfer->event = event;
    if (event == I2C_SLAVE_RECEIVE) i2c_read_raw_blocking(i2c, &transfer->data, 1);
}



void init_mitm() {
    mitm_transfer_queue = create_static_queue(MITM_QUEUE_MAX_ELEMENTS, MITM_QUEUE_ELEMENT_SIZE);

    i2c_dev_t* laptop = get_laptop_dev();
    i2c_slave_init(laptop->i2c, laptop->address, &mitm_laptop_irq_handler);
}


void mitm_loop() {
    i2c_dev_t* laptop = get_laptop_dev();
    i2c_dev_t* bms = get_bms_dev();

    smbus_transfer_t* transfer = static_queue_peek(mitm_transfer_queue);
    if (transfer == NULL) return;

    status_mitm(true);

    while (transfer != NULL) {

        switch (transfer->event) {
            case I2C_SLAVE_RECEIVE:

                // this should never happen
                if (previous_event == I2C_SLAVE_REQUEST) {
                    printf("ERROR: no stop after read before write!!!\n");
                    mitm_reply_buffer[0] = 0;
                    i2c_write_raw_blocking(laptop->i2c, mitm_reply_buffer, 1);
                    break;
                }

                if (mitm_cmd_buffer_index + 1 >= MITM_CMD_BUFFER_SIZE) {
                    printf("ERROR: cmd buffer overrun!!!\n");
                    break;
                }
                
                // write previous byte
                if (mitm_cmd_buffer_index > 0) {
                    i2c_write_burst_blocking(bms->i2c, bms->address, &mitm_cmd_buffer[mitm_cmd_buffer_index - 1], 1);
                }

                mitm_cmd_buffer[mitm_cmd_buffer_index++] = transfer->data;

                printf("TX 0x%02x\n", mitm_cmd_buffer[mitm_cmd_buffer_index-1]);

                break;
            case I2C_SLAVE_REQUEST:

                // this should never happen
                if (previous_event == I2C_SLAVE_RECEIVE) {
                    printf("ERROR: no stop after write before read!!!\n");
                    mitm_reply_buffer[0] = 0;
                    i2c_write_raw_blocking(laptop->i2c, mitm_reply_buffer, 1);
                    break;
                }

                if (mitm_reply_buffer_index + 1 >= MITM_REPLY_BUFFER_SIZE) {
                    printf("ERROR: reply buffer overrun!!!\n");
                    mitm_reply_buffer[0] = 0;
                    i2c_write_raw_blocking(laptop->i2c, mitm_reply_buffer, 1);
                    break;
                }

                // forward reply from bms
                i2c_read_burst_blocking(bms->i2c, bms->address, &mitm_reply_buffer[mitm_reply_buffer_index], 1);
                i2c_write_raw_blocking(laptop->i2c, &mitm_reply_buffer[mitm_reply_buffer_index], 1);

                printf("RX 0x%02x\n", mitm_reply_buffer[mitm_reply_buffer_index]);

                mitm_reply_buffer_index++;
                break;
            case I2C_SLAVE_FINISH:
                //todo: need to replace the actual interrupt if I want this to be accurate, this impl is just guessing
                if (previous_event == I2C_SLAVE_RECEIVE) {

                    bool is_cmd = mitm_cmd_buffer_index == 1;
                    i2c_write_blocking(bms->i2c, bms->address, mitm_cmd_buffer + mitm_cmd_buffer_index - 1, 1, is_cmd); // cmd expects rx
                    if (is_cmd) {
                        printf("end of cmd\n");
                    } else {
                        printf("end of write (%d bytes)\n", mitm_cmd_buffer_index);
                    }

                } else if (previous_event == I2C_SLAVE_REQUEST) {
                    i2c_stop_blocking(bms);
                    printf("end of read (%d bytes)\n", mitm_reply_buffer_index);
                } else {
                    printf("RESET\n");
                }
                
                mitm_cmd_buffer_index = 0;
                mitm_reply_buffer_index = 0;
                break;
            default:
                printf("event invalid 0x%02x\n", transfer->event);
                break;
        }
        
        previous_event = transfer->event;
        static_queue_pop(mitm_transfer_queue);
        transfer = static_queue_peek(mitm_transfer_queue);
    } 

    status_mitm(false);

}



