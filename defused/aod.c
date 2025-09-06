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
#include "aod.h"
#include "display.h"
#include "battery.h"

battery_stat_t* aod_charge;
battery_stat_t* aod_voltage;
battery_stat_t* aod_current;
battery_stat_t* aod_remaining_capacity;


bool defused_aod_on_button_event(button_func_t function, button_event_t event) { return true; }

void defused_aod_on_nav_up() {}
void defused_aod_on_nav_down() {}
void defused_aod_on_pre_select() {}
void defused_aod_on_cancel_select() {}
void defused_aod_on_select() {}
bool defused_aod_on_select_held() { return false; }

void aod_update_stats() {
    battery_stat_request_update(aod_charge);
    battery_stat_request_update(aod_voltage);
    battery_stat_request_update(aod_current);
    battery_stat_request_update(aod_remaining_capacity);
}

bool aod_print_stat_error(battery_stat_t* stat) {
    if (battery_stat_is_expired(stat)) {
        display_set_text_color(COLOR_GRAY);
        display_print("...");
        return true;
    } else if (battery_stat_is_error(stat)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
        return true;
    }
    return false;
}

void defused_aod_update_display() {
    float current;
    float voltage;
    aod_update_stats();


    display_clear();



    battery_stat_lock();

    display_set_text_position(0, 24);
    display_set_text_scale(2);

    if (!aod_print_stat_error(aod_charge)) {
        display_set_text_color(COLOR_WHITE);
        display_printf("%d%%", *aod_charge->cached_result.as_uint16);
    }
    
    display_set_text_position(0, 34);
    display_set_text_scale(1);

    if (!aod_print_stat_error(aod_remaining_capacity)) {
        display_set_text_color(COLOR_GRAY);
        display_printf("%.2f Wh", (*aod_remaining_capacity->cached_result.as_uint16) / 100.0);
    }

    display_set_text_position(0, 54);

    if (!aod_print_stat_error(aod_voltage)) {
        display_set_text_color(COLOR_GREEN);
        voltage = (*aod_voltage->cached_result.as_uint16) / 1000.0;
        if (voltage < 10.0) display_print(" ");
        display_printf("%.2f V", voltage);
    }

    display_print(" ");

    if (!aod_print_stat_error(aod_current)) {
        display_set_text_color(COLOR_RED);
        current = (*aod_current->cached_result.as_int16) / 1000.0;
        if (current >= 0.0) display_print("+");
        display_printf("%.2f A", current);
    }

    battery_stat_unlock();



    display_refresh();
    display_burn_update(true);

}

void defused_aod_init() {
    aod_charge = battery_get_stat(BATT_CMD_RELATIVE_STATE_OF_CHARGE);
    aod_voltage = battery_get_stat(BATT_CMD_VOLTAGE);
    aod_current = battery_get_stat(BATT_CMD_CURRENT);
    aod_remaining_capacity = battery_get_stat(BATT_CMD_REMAINING_CAPACITY);
    
    aod_update_stats();
}


menu_binding_t defused_aod_menu_binding = {
    display_update_interval: 1000000,
    burn_margin_x: 5,
    burn_margin_y: 10,

    on_button_event: &defused_aod_on_button_event,

    on_nav_up: &defused_aod_on_nav_up,
    on_nav_down: &defused_aod_on_nav_down,
    on_pre_select: &defused_aod_on_pre_select,
    on_cancel_select: &defused_aod_on_cancel_select,
    on_select: &defused_aod_on_select,
    on_select_held: &defused_aod_on_select_held,
    
    update_display: &defused_aod_update_display,
    
    init: &defused_aod_init
};

menu_binding_t* bind_aod() {
    return &defused_aod_menu_binding;
}
