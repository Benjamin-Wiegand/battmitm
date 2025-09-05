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
#include <stdint.h>
#include <stdbool.h>

#define BATT_CMD_MANUFACTURER_ACCESS 0x00
#define BATT_CMD_REMAINING_CAPACITY_ALARM 0x01
#define BATT_CMD_REMAINING_TIME_ALARM 0x02

#define BATT_CMD_BATTERY_MODE 0x03
#define BATT_CMD_AT_RATE 0x04
#define BATT_CMD_AT_RATE_TIME_TO_FULL 0x05
#define BATT_CMD_AT_RATE_TIME_TO_EMPTY 0x06
#define BATT_CMD_AT_RATE_OK 0x07

#define BATT_CMD_TEMPERATURE 0x08
#define BATT_CMD_VOLTAGE 0x09
#define BATT_CMD_CURRENT 0x0a
#define BATT_CMD_AVERAGE_CURRENT 0x0b

#define BATT_CMD_MAX_ERROR 0x0c
#define BATT_CMD_RELATIVE_STATE_OF_CHARGE 0x0d
#define BATT_CMD_ABSOLUTE_STATE_OF_CHARGE 0x0e
#define BATT_CMD_REMAINING_CAPACITY 0x0f
#define BATT_CMD_FULL_CHARGE_CAPACITY 0x10

#define BATT_CMD_RUN_TIME_TO_EMPTY 0x11
#define BATT_CMD_AVERAGE_TIME_TO_EMPTY 0x12
#define BATT_CMD_AVERAGE_TIME_TO_FULL 0x13

#define BATT_CMD_CHARGING_CURRENT 0x14
#define BATT_CMD_CHARGING_VOLTAGE 0x15

#define BATT_CMD_BATTERY_STATUS 0x16
#define BATT_CMD_CYCLE_COUNT 0x17
#define BATT_CMD_DESIGN_CAPACITY 0x18
#define BATT_CMD_DESIGN_VOLTAGE 0x19
#define BATT_CMD_SPECIFICATION_INFO 0x1a
#define BATT_CMD_MANUFACTURE_DATE 0x1b
#define BATT_CMD_SERIAL_NUMBER 0x1c

#define BATT_CMD_MANUFACTURER_NAME 0x20
#define BATT_CMD_DEVICE_NAME 0x21
#define BATT_CMD_DEVICE_CHEMISTRY 0x22
#define BATT_CMD_MANUFACTURER_DATA 0x23


#define BATTERY_STAT_VALID_PERIOD_DEFAULT 5000000       // 5 sec
#define BATTERY_STAT_VALID_PERIOD_CONSTANT 1200000000   // 20 min
#define BATTERY_STAT_MIN_RETRY_PERIOD 3000000           // 3 sec


enum battery_stat_type {
    SBS_BYTES,
    SBS_BLOCK,
    SBS_STRING
};

typedef enum battery_stat_type battery_stat_type_t;

union battery_response {
    uint16_t* as_uint16;
    uint8_t* as_uint8;
    int16_t* as_int16;
    char* as_string;
};

typedef union battery_response battery_response_t;

struct battery_stat {
    uint8_t read_command;
    char* friendly_name;
    uint8_t max_result_length;
    uint32_t valid_for;
    battery_stat_type_t type;

    battery_response_t cached_result;
    uint8_t result_length;
    bool result_valid;
    uint64_t last_updated;

    bool update_requested;
};

typedef struct battery_stat battery_stat_t;

// IMPORTANT:
// - listen to the warnings in the comments below when calling these functions
// - accessing the battery_stat struct from core1 safely requires a lock
// - make it quick, long locks will delay comms between the laptop and battery and could cause problems

// only use these on core1, never in an interrupt
// (doesn't apply to internal functions)
void battery_stat_lock();
void battery_stat_unlock();

// needs lock (on core1)
bool battery_stat_is_error(battery_stat_t* batt_stat);
bool battery_stat_is_expired(battery_stat_t* batt_stat);

// thread safe (ish)
battery_stat_t* battery_get_stat(uint8_t cmd);  // note: the struct at the pointer is not thread-safe!
void battery_stat_request_update(battery_stat_t* batt_stat);

// only call from core0, never in an interrupt
void battery_update_cache();

void init_battery();
