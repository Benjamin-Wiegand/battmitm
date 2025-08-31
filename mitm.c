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
#include "static_queue.h"
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include <stdio.h>
#include "config_override.h"

enum i2c_transfer_event {
    I2C_READ,
    I2C_WRITE,
    I2C_START,
    I2C_STOP,
    I2C_ABORT
};

typedef enum i2c_transfer_event i2c_transfer_event_t;

struct i2c_transfer {
    i2c_transfer_event_t event;
    uint8_t data;
};

typedef struct i2c_transfer i2c_transfer_t;

#define MITM_QUEUE_ELEMENT_SIZE sizeof(i2c_transfer_t)

static_queue_t* mitm_transfer_queue;
bool mitm_transfer_queue_overflow = false;

i2c_transfer_event_t previous_event = I2C_ABORT;

uint8_t mitm_cmd_buffer[MITM_CMD_BUFFER_SIZE];
size_t mitm_cmd_buffer_index = 0;

uint8_t mitm_reply_buffer[MITM_REPLY_BUFFER_SIZE];
size_t mitm_reply_buffer_index = 0;
bool reply_override = false;


void mitm_laptop_on_i2c_event(i2c_transfer_event_t event) {
    i2c_transfer_t* transfer = static_queue_add(mitm_transfer_queue);
    if (transfer == NULL) {
        mitm_transfer_queue_overflow = true;
        return;
    }

    transfer->event = event;
    if (event == I2C_WRITE) i2c_read_raw_blocking(LAPTOP_I2C, &transfer->data, 1);
}

void mitm_laptop_irq_handler() {
    i2c_hw_t* hw = i2c_get_hw(LAPTOP_I2C);
    uint32_t stat = hw->intr_stat;
    if (stat == 0) return;
    if (mitm_transfer_queue_overflow) return;

    if (stat & I2C_IC_INTR_STAT_R_TX_ABRT_BITS) {
        hw->clr_tx_abrt;
        mitm_laptop_on_i2c_event(I2C_ABORT);
    }

    if (stat & I2C_IC_INTR_STAT_R_START_DET_BITS) {
        hw->clr_start_det;
        mitm_laptop_on_i2c_event(I2C_START);
    }

    if (stat & I2C_IC_INTR_STAT_R_STOP_DET_BITS) {
        hw->clr_stop_det;
        mitm_laptop_on_i2c_event(I2C_STOP);
    }


    if (stat & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
        mitm_laptop_on_i2c_event(I2C_WRITE);
    }

    if (stat & I2C_IC_INTR_STAT_R_RD_REQ_BITS) {
        hw->clr_rd_req;
        mitm_laptop_on_i2c_event(I2C_READ);
    }
}


void mitm_init_i2c() {
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

    // manually set up irq
    uint i2c_irq = I2C0_IRQ + i2c_hw_index(LAPTOP_I2C);
    i2c_get_hw(LAPTOP_I2C)->intr_mask = I2C_IC_INTR_MASK_M_RX_FULL_BITS | I2C_IC_INTR_MASK_M_RD_REQ_BITS | I2C_IC_INTR_MASK_M_TX_ABRT_BITS | I2C_IC_INTR_MASK_M_STOP_DET_BITS | I2C_IC_INTR_MASK_M_START_DET_BITS;
    irq_set_exclusive_handler(i2c_irq, &mitm_laptop_irq_handler);
    irq_set_enabled(i2c_irq, true);
}

void mitm_reinit_i2c() {
    uint i2c_irq = I2C0_IRQ + i2c_hw_index(LAPTOP_I2C);
    irq_set_enabled(i2c_irq, false);
    irq_remove_handler(i2c_irq, &mitm_laptop_irq_handler);
    i2c_get_hw(LAPTOP_I2C)->intr_mask = I2C_IC_INTR_MASK_RESET;

    i2c_set_slave_mode(LAPTOP_I2C, false, LAPTOP_I2C_ADDR);
    i2c_deinit(BATT_I2C);
    i2c_deinit(LAPTOP_I2C);

    sleep_ms(1000);
    mitm_transfer_queue_overflow = false;

    mitm_init_i2c();
}


int mitm_read_batt_reply(uint8_t* buffer, size_t length) {
    return i2c_read_burst_blocking(BATT_I2C, BATT_I2C_ADDR, buffer, length);
}

bool mitm_validate_batt_reply(uint8_t* buffer, uint8_t crc_index, bool is_block) {
    if (mitm_cmd_buffer_index != 1) return false;               // not a reply
    if (is_block && crc_index < 1) return false;                // block requires at least one byte for the length
    if (is_block && buffer[0] != crc_index - 1) return false;   // block length does not match crc index

    return validate_smbus_crc(
        BATT_I2C_ADDR, mitm_cmd_buffer[0], 
        is_block ? &buffer[1] : buffer, 
        is_block ? buffer[0] : crc_index, 
        buffer[crc_index], is_block, true);
}

int mitm_generate_reply_crc(uint8_t* buffer, uint8_t crc_index, bool is_block) {
    if (mitm_cmd_buffer_index != 1) return -1;              // not a reply
    if (is_block && crc_index < 1) return -1;               // block requires at least one byte for the length
    if (is_block && buffer[0] != crc_index - 1) return -1;  // block length does not match crc index

    buffer[crc_index] = generate_smbus_crc(
        BATT_I2C_ADDR, mitm_cmd_buffer[0], 
        is_block ? &buffer[1] : buffer,
        is_block ? buffer[0] : crc_index, 
        is_block, true);
    return 0;
}


void init_mitm() {
    mitm_transfer_queue = create_static_queue(MITM_QUEUE_MAX_ELEMENTS, MITM_QUEUE_ELEMENT_SIZE);
    mitm_init_i2c();
}


void mitm_loop() {
    i2c_dev_t* laptop = get_laptop_dev();
    i2c_dev_t* bms = get_bms_dev();

    i2c_transfer_t* transfer = static_queue_peek(mitm_transfer_queue);

    if (mitm_transfer_queue_overflow) {
        printf("ERROR: mitm transfer queue overflow!!! please increase MITM_QUEUE_MAX_ELEMENTS\n");
        
        // flush the queue
        do {
            transfer = static_queue_pop(mitm_transfer_queue);
        } while (transfer != NULL);

        // drop off the bus temporarily
        mitm_reinit_i2c();
    }

    while (transfer != NULL) {
        status_mitm(true);
        
        switch (transfer->event) {
            case I2C_WRITE:

                // this should never happen
                if (previous_event == I2C_READ) {
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
            case I2C_READ:

                // this should never happen
                if (previous_event == I2C_WRITE) {
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

                if (reply_override) {
                    // forward modified reply
                    i2c_write_raw_blocking(laptop->i2c, &mitm_reply_buffer[mitm_reply_buffer_index], 1);
                } else {
                    // forward reply from bms
                    i2c_read_burst_blocking(bms->i2c, bms->address, &mitm_reply_buffer[mitm_reply_buffer_index], 1);
                    i2c_write_raw_blocking(laptop->i2c, &mitm_reply_buffer[mitm_reply_buffer_index], 1);
                }

                printf("RX 0x%02x\n", mitm_reply_buffer[mitm_reply_buffer_index]);

                mitm_reply_buffer_index++;
                break;
            case I2C_ABORT:
            case I2C_STOP:
                bool aborted = transfer->event == I2C_ABORT;

                if (previous_event == I2C_WRITE) {
                    i2c_write_blocking(bms->i2c, bms->address, mitm_cmd_buffer + mitm_cmd_buffer_index - 1, 1, false);
                    if (aborted) printf("ABORT - ");
                    printf("end of TX (%d bytes)\n", mitm_cmd_buffer_index);
                } else if (previous_event == I2C_READ) {
                    i2c_stop_blocking(bms);     // pay the court a fine or serve your sentence
                    if (aborted) printf("ABORT - ");
                    printf("end of RX (%d bytes)\n", mitm_reply_buffer_index);
                } else {
                    if (aborted) printf("ABORT\n");
                    else printf("STOP\n");
                }

                mitm_cmd_buffer_index = 0;
                mitm_reply_buffer_index = 0;
                reply_override = false;
                break;
            case I2C_START:
                reply_override = false;

                if (previous_event == I2C_WRITE) {
                    i2c_write_blocking(bms->i2c, bms->address, mitm_cmd_buffer + mitm_cmd_buffer_index - 1, 1, true);
                    printf("switching TX -> RX after sending (%d bytes)\n", mitm_cmd_buffer_index);

                    // apply read command overrides
                    if (mitm_cmd_buffer_index == 1) { // read command
                        cmd_reply_override override = get_read_command_reply_override(mitm_cmd_buffer[0]);
                        if (override != NULL) {
                            reply_override = true;
                            printf("read command reply override!\n");
                            int ret = override(mitm_cmd_buffer[0], mitm_reply_buffer);
                            if (ret < 0) {
                                printf("read command reply override returned %d, trashing response\n", ret);
                                // since the slave can't abort the transfer, this is the best we can do
                                for (int i = 0; i < MITM_REPLY_BUFFER_SIZE; i++) {
                                    mitm_reply_buffer[i] = 0;
                                }
                            }
                        }
                    }

                } else if (previous_event == I2C_READ) {
                    printf("START - end of RX (%d bytes)\n", mitm_reply_buffer_index);
                } else {
                    printf("START\n");
                }

                mitm_cmd_buffer_index = 0;
                mitm_reply_buffer_index = 0;
                break;
            default:
                break;
        }
        
        previous_event = transfer->event;
        static_queue_pop(mitm_transfer_queue);
        transfer = static_queue_peek(mitm_transfer_queue);
    } 

    status_mitm(false);

}

