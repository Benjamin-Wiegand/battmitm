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
#include "defused/stat_browser.h"
#include "display.h"
#include "font.h"
#include "battery.h"
#include <stdlib.h>

typedef void (*stat_browser_display_page_callback_t)();

battery_stat_t* stat_browser_charge;
battery_stat_t* stat_browser_max_error;
battery_stat_t* stat_browser_voltage;
battery_stat_t* stat_browser_current;
battery_stat_t* stat_browser_temperature;
battery_stat_t* stat_browser_remaining_capacity;

battery_stat_t* stat_browser_full_capacity;
battery_stat_t* stat_browser_design_capacity;
battery_stat_t* stat_browser_cycle_count;
battery_stat_t* stat_browser_manufacture_date;

stat_browser_display_page_callback_t* stat_browser_page_callbacks = NULL;
size_t stat_browser_page_callbacks_size = 0;
size_t stat_browser_page_index = 0;

bool defused_stat_browser_on_button_event(button_func_t function, button_event_t event) {
    return false;
}

void defused_stat_browser_on_nav_up() {
    if (stat_browser_page_index == 0) return;
    stat_browser_page_index--;
    defused_update_display_now();
}

void defused_stat_browser_on_nav_down() {
    if (stat_browser_page_index >= stat_browser_page_callbacks_size - 1) return;
    stat_browser_page_index++;
    defused_update_display_now();
}

void defused_stat_browser_on_pre_select() {
}

void defused_stat_browser_on_cancel_select() {
}

void defused_stat_browser_on_select() {
}

bool defused_stat_browser_on_select_held() {
    defused_enter_inactive_mode();
    return true;
}


void defused_stat_browser_display_general_info() {
    uint16_t max_error;
    float voltage;
    float current;
    float temperature;
    float wattage;
    battery_stat_request_update(stat_browser_charge);
    battery_stat_request_update(stat_browser_voltage);
    battery_stat_request_update(stat_browser_current);
    battery_stat_request_update(stat_browser_temperature);
    battery_stat_request_update(stat_browser_remaining_capacity);
    

    // page name
    display_set_text_position(0, display_area_height());
    display_set_text_scale(1);
    display_set_text_color(COLOR_WHITE);
    display_print("general");


    battery_stat_lock();

    if (battery_stat_is_valid(stat_browser_max_error)) {
        max_error = *stat_browser_max_error->cached_result.as_uint16;
    } else {
        max_error = 0;
    }

    // battery %
    display_set_text_position(0, 20);
    display_set_text_scale(2);
    if (battery_stat_is_expired(stat_browser_charge)) {
        display_set_text_color(COLOR_GRAY);
        display_print("--%");
    } else if (battery_stat_is_error(stat_browser_charge)) {
        display_set_text_color(COLOR_RED);
        display_print("ERR%");
    } else {
        display_set_text_color(COLOR_WHITE);
        display_printf("%d%%", (*stat_browser_charge->cached_result.as_uint16) + (max_error / 2));
    }

    display_set_text_scale(1);
    
    // battery % error
    display_print(" ");
    if (max_error != 0) {
        display_set_text_color(COLOR_GRAY);
        display_printf("+/-%d%%", max_error);
    }
    
    // battery remaining capacity
    display_set_text_position(0, 30);
    if (battery_stat_is_expired(stat_browser_remaining_capacity)) {
        display_set_text_color(COLOR_GRAY);
        display_print(" -.-- Wh");
    } else if (battery_stat_is_error(stat_browser_remaining_capacity)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
    } else {
        display_set_text_color(COLOR_GRAY);
        display_printf("%.2f Wh", (*stat_browser_remaining_capacity->cached_result.as_uint16) / 100.0);
    }

    // battery voltage
    display_set_text_position(0, 40);
    if (battery_stat_is_expired(stat_browser_voltage)) {
        display_set_text_color(COLOR_GRAY);
        display_print(" -.-- V");
    } else if (battery_stat_is_error(stat_browser_voltage)) {
        display_set_text_color(COLOR_RED);
        display_print("error  ");
    } else {
        display_set_text_color(COLOR_GREEN);
        voltage = (*stat_browser_voltage->cached_result.as_uint16) / 1000.0;
        if (voltage < 10.0) display_print(" ");
        display_printf("%.2f V", voltage);
    }
    
    // battery current
    display_print(" ");
    if (battery_stat_is_expired(stat_browser_current)) {
        display_set_text_color(COLOR_GRAY);
        display_print(" -.-- A");
    } else if (battery_stat_is_error(stat_browser_current)) {
        display_set_text_color(COLOR_RED);
        display_print("error  ");
    } else {
        display_set_text_color(COLOR_RED);
        current = (*stat_browser_current->cached_result.as_int16) / 1000.0;
        if (current >= 0.0) display_print("+");            
        display_printf("%.2f A", current);
    }
    
    // battery temperature
    display_set_text_position(0, 50);
    if (battery_stat_is_expired(stat_browser_temperature)) {
        display_set_text_color(COLOR_GRAY);
        display_print("---.- K");
    } else if (battery_stat_is_error(stat_browser_temperature)) {
        display_set_text_color(COLOR_RED);
        display_print("error  ");
    } else {
        display_set_text_color(COLOR_BLUE);
        temperature = (*stat_browser_temperature->cached_result.as_uint16) / 10.0;
        display_printf("%.1f K", temperature);
    }
    
    // wattage
    display_print(" ");
    if (battery_stat_is_valid(stat_browser_voltage) && battery_stat_is_valid(stat_browser_current)) {
        display_set_text_color(COLOR_GRAY);
        wattage = voltage * current;
        if (wattage >= 0.0) display_print("+");            
        display_printf("%.1f W", wattage);
    } else {
        display_printf(" --.- W");
    }

    battery_stat_unlock();
}

void defused_stat_browser_display_health_info() {
    float full_capacity;
    float design_capacity;
    float calculated_health;
    float temperature;
    uint16_t cycle_count;
    battery_stat_request_update(stat_browser_temperature);
    battery_stat_request_update(stat_browser_full_capacity);
    battery_stat_request_update(stat_browser_design_capacity);
    battery_stat_request_update(stat_browser_cycle_count);
    battery_stat_request_update(stat_browser_manufacture_date); //TODO


    // page name
    display_set_text_position(0, display_area_height());
    display_set_text_scale(1);
    display_set_text_color(COLOR_WHITE);
    display_print("health");


    battery_stat_lock();

    // battery full capacity
    display_set_text_position(0, 10);
    display_set_text_scale(1);
    if (battery_stat_is_expired(stat_browser_full_capacity)) {
        display_set_text_color(COLOR_GRAY);
        display_print("--.-/--.- Wh");
    } else if (battery_stat_is_error(stat_browser_full_capacity)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
    } else {
        display_set_text_color(COLOR_WHITE);
        full_capacity = (*stat_browser_full_capacity->cached_result.as_uint16) / 100.0;
        display_printf("%.1f", full_capacity);

        // battery design capacity
        if (!battery_stat_is_valid(stat_browser_design_capacity)) {
            display_print(" Wh");
            display_set_text_position(0, 20);

            display_print("wear: ");
            display_set_text_color(COLOR_RED);
            display_print(" ?%");
        } else {
            display_set_text_color(COLOR_GRAY);
            design_capacity = (*stat_browser_design_capacity->cached_result.as_uint16) / 100.0;
            display_printf("/%.1f Wh", design_capacity);
            
            // health
            display_set_text_position(0, 20);
            display_set_text_color(COLOR_WHITE);
            display_print("wear: ");
            display_set_text_color(COLOR_RED);
            calculated_health = 100 * full_capacity / design_capacity;
            display_printf("%.3f%%", 100.0 - calculated_health);
        }
    }
    
    // battery cycle count
    display_set_text_position(0, 30);
    display_set_text_color(COLOR_WHITE);
    display_print("cycl: ");
    if (battery_stat_is_expired(stat_browser_cycle_count)) {
        display_set_text_color(COLOR_GRAY);
        display_print("---");
    } else if (battery_stat_is_error(stat_browser_cycle_count)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
    } else {
        display_set_text_color(COLOR_GRAY);
        cycle_count = *stat_browser_cycle_count->cached_result.as_uint16;
        display_printf("%d", cycle_count);
    }
    
    // battery temperature
    display_set_text_position(0, 40);
    display_set_text_color(COLOR_WHITE);
    display_print("temp: ");
    if (battery_stat_is_expired(stat_browser_temperature)) {
        display_set_text_color(COLOR_GRAY);
        display_print("---.- K");
    } else if (battery_stat_is_error(stat_browser_temperature)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
    } else {
        display_set_text_color(COLOR_BLUE);
        temperature = (*stat_browser_temperature->cached_result.as_uint16) / 10.0;
        display_printf("%.1f K", temperature);
    }

    battery_stat_unlock();
}


void defused_stat_browser_update_display() {
    display_clear();

    // page number
    display_set_text_position(display_area_width() - FONT_WIDTH, display_area_height());
    display_set_text_scale(1);
    display_set_text_color(COLOR_WHITE);
    display_printf("%d", stat_browser_page_index);
    
    stat_browser_page_callbacks[stat_browser_page_index]();
    
    display_refresh();
    display_burn_update(true);
}

void defused_stat_browser_init() {
    if (stat_browser_page_callbacks == NULL) {
        stat_browser_display_page_callback_t page_callbacks_init[] = {
            &defused_stat_browser_display_general_info,
            &defused_stat_browser_display_health_info,
        };
        stat_browser_page_callbacks_size = sizeof(page_callbacks_init) / sizeof(stat_browser_display_page_callback_t);
        stat_browser_page_callbacks = malloc(sizeof(page_callbacks_init));
        for (int i = 0; i < stat_browser_page_callbacks_size; i++) {
            stat_browser_page_callbacks[i] = page_callbacks_init[i];
        }
    }

    stat_browser_charge = battery_get_stat(BATT_CMD_RELATIVE_STATE_OF_CHARGE);
    stat_browser_voltage = battery_get_stat(BATT_CMD_VOLTAGE);
    stat_browser_current = battery_get_stat(BATT_CMD_CURRENT);
    stat_browser_temperature = battery_get_stat(BATT_CMD_TEMPERATURE);
    stat_browser_remaining_capacity = battery_get_stat(BATT_CMD_REMAINING_CAPACITY);
    stat_browser_full_capacity = battery_get_stat(BATT_CMD_FULL_CHARGE_CAPACITY);
    stat_browser_design_capacity = battery_get_stat(BATT_CMD_DESIGN_CAPACITY);
    stat_browser_cycle_count = battery_get_stat(BATT_CMD_CYCLE_COUNT);
    stat_browser_manufacture_date = battery_get_stat(BATT_CMD_MANUFACTURE_DATE);
}


menu_binding_t defused_stat_browser_menu_binding = {
    display_update_interval: 1000000,
    burn_margin_x: 4,
    burn_margin_y: 4,

    on_button_event: &defused_stat_browser_on_button_event,

    on_nav_up: &defused_stat_browser_on_nav_up,
    on_nav_down: &defused_stat_browser_on_nav_down,
    on_pre_select: &defused_stat_browser_on_pre_select,
    on_cancel_select: &defused_stat_browser_on_cancel_select,
    on_select: &defused_stat_browser_on_select,
    on_select_held: &defused_stat_browser_on_select_held,
    
    update_display: &defused_stat_browser_update_display,
    
    init: &defused_stat_browser_init
};

menu_binding_t* bind_stat_browser() {
    return &defused_stat_browser_menu_binding;
}
