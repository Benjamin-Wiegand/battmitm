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
#include "battery.h"
#include "config.h"
#include "smbus.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdio.h>


bool battery_stat_need_cache_update = false;

spin_lock_t* battery_stat_cache_lock;
battery_stat_t* battery_stat_cache;
size_t battery_stat_cache_size;


void battery_stat_lock() {
    spin_lock_unsafe_blocking(battery_stat_cache_lock);
}

void battery_stat_unlock() {
    spin_unlock_unsafe(battery_stat_cache_lock);
}

bool battery_stat_is_error(battery_stat_t* batt_stat) {
    return !batt_stat->result_valid;
}


bool battery_stat_is_expired(battery_stat_t* batt_stat) {
    return batt_stat->last_updated == 0 || batt_stat->last_updated + batt_stat->valid_for < time_us_64();
}


battery_stat_t* battery_get_stat(uint8_t cmd) {
    // todo: this can be optimized
    battery_stat_t* batt_stat;
    for (int i = 0; i < battery_stat_cache_size; i++) {
        batt_stat = &battery_stat_cache[i];
        if (batt_stat->read_command != cmd) continue;
        batt_stat->update_requested = true;
        battery_stat_need_cache_update = true;
        return batt_stat;
    }
}


void battery_update_cache() {
    if (!battery_stat_need_cache_update) return;
    
    battery_stat_lock();

    battery_stat_need_cache_update = false;

    i2c_dev_t* bms = get_bms_dev();
    battery_stat_t* batt_stat;
    int ret;

    for (int i = 0; i < battery_stat_cache_size; i++) {
        batt_stat = &battery_stat_cache[i];
        if (!batt_stat->update_requested) continue;
        if (batt_stat->last_updated + BATTERY_STAT_MIN_RETRY_PERIOD > time_us_64()) continue;

        printf("update requested for %02x (%s)\n", batt_stat->read_command, batt_stat->friendly_name);

        switch (batt_stat->type) {
            case SBS_BYTES:
                ret = smbus_read(bms, batt_stat->read_command, batt_stat->cached_result, batt_stat->max_result_length);
                break;
            case SBS_BLOCK:
                ret = smbus_read_block(bms, batt_stat->read_command, batt_stat->cached_result, batt_stat->max_result_length);
                break;
            case SBS_STRING:
                ret = smbus_read_text(bms, batt_stat->read_command, batt_stat->cached_result, batt_stat->max_result_length);
                break;
            default:
                continue;
        }

        if (ret < 0) {
            batt_stat->result_valid = false;
        } else {
            batt_stat->result_valid = true;
            batt_stat->result_length = ret;
            batt_stat->update_requested = false;
        }
        
        batt_stat->last_updated = time_us_64();
    }

    battery_stat_unlock();
}


battery_stat_t create_battery_stat(uint8_t cmd, char* friendly_name, uint8_t max_result_length, battery_stat_type_t type, uint32_t valid_for) {
    battery_stat_t batt_stat = {
        read_command: cmd,
        friendly_name: friendly_name,
        max_result_length: max_result_length,
        valid_for: valid_for,
        type: type,

        cached_result: malloc(max_result_length),
        result_length: 0,
        result_valid: false,
        last_updated: 0,

        update_requested: false
    };
    
    return batt_stat;
}


void init_battery() {
    battery_stat_t battery_stat_cache_init[] = {
        create_battery_stat(BATT_CMD_MANUFACTURER_ACCESS, "manufacturer access", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_REMAINING_CAPACITY_ALARM, "capacity alarm", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_REMAINING_TIME_ALARM, "time alarm", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_BATTERY_MODE, "battery mode", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_AT_RATE, "at rate", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_AT_RATE_TIME_TO_FULL, "time to full", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_AT_RATE_TIME_TO_EMPTY, "time to empty", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_AT_RATE_OK, "rate ok", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_TEMPERATURE, "temperature", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_VOLTAGE, "voltage", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_CURRENT, "current", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_AVERAGE_CURRENT, "avg current", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_MAX_ERROR, "max error", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_RELATIVE_STATE_OF_CHARGE, "relative charge", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_ABSOLUTE_STATE_OF_CHARGE, "absolute charge", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_REMAINING_CAPACITY, "remaining", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_FULL_CHARGE_CAPACITY, "full capacity", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_RUN_TIME_TO_EMPTY, "run time", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_AVERAGE_TIME_TO_EMPTY, "avg run time", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_AVERAGE_TIME_TO_FULL, "avg charge time", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_CHARGING_CURRENT, "charge current", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_CHARGING_VOLTAGE, "charge voltage", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_BATTERY_STATUS, "battery status", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),
        create_battery_stat(BATT_CMD_CYCLE_COUNT, "cycles", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_DEFAULT),

        create_battery_stat(BATT_CMD_DESIGN_CAPACITY, "design capacity", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_CONSTANT),
        create_battery_stat(BATT_CMD_DESIGN_VOLTAGE, "design voltage", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_CONSTANT),
        create_battery_stat(BATT_CMD_SPECIFICATION_INFO, "spec info", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_CONSTANT),
        create_battery_stat(BATT_CMD_MANUFACTURE_DATE, "manufacture date", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_CONSTANT),
        create_battery_stat(BATT_CMD_SERIAL_NUMBER, "serial no.", 2, SBS_BYTES, BATTERY_STAT_VALID_PERIOD_CONSTANT),

        create_battery_stat(BATT_CMD_MANUFACTURER_NAME, "manufacturer", 32, SBS_STRING, BATTERY_STAT_VALID_PERIOD_CONSTANT),
        create_battery_stat(BATT_CMD_DEVICE_NAME, "device name", 32, SBS_STRING, BATTERY_STAT_VALID_PERIOD_CONSTANT),
        create_battery_stat(BATT_CMD_DEVICE_CHEMISTRY, "chemistry", 16, SBS_STRING, BATTERY_STAT_VALID_PERIOD_CONSTANT),
        create_battery_stat(BATT_CMD_MANUFACTURER_DATA, "manufacturer data", 14, SBS_BLOCK, BATTERY_STAT_VALID_PERIOD_CONSTANT),
    };
    battery_stat_cache_size = sizeof(battery_stat_cache_init) / sizeof(battery_stat_t);
    battery_stat_cache = malloc(sizeof(battery_stat_cache_init));
    for (int i = 0; i < battery_stat_cache_size; i++) {
        battery_stat_cache[i] = battery_stat_cache_init[i];
    }

    battery_stat_cache_lock = spin_lock_instance(spin_lock_claim_unused(true));
}
