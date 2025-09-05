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

// this mechanism will be replaced
#define SCREEN_DIM_TIMEOUT 5000000

uint64_t aod_last_input = 0;

bool defused_aod_on_button_event(button_func_t function, button_event_t event) {
    aod_last_input = time_us_64();
    defused_update_display_now();
    return true;
}

void defused_aod_on_nav_up() {}
void defused_aod_on_nav_down() {}
void defused_aod_on_pre_select() {}
void defused_aod_on_cancel_select() {}
void defused_aod_on_select() {}
bool defused_aod_on_select_held() { return false; }


void defused_aod_update_display() {

    display_burn_update(false);
    
    display_clear();

    if (aod_last_input + SCREEN_DIM_TIMEOUT < time_us_64()) {
        display_set_contrast(0); // dimmest setting
    } else {
        display_set_contrast(255);
    }


    battery_stat_lock();
    battery_stat_t* stat;

    display_set_text_position(0, 36);
    display_set_text_scale(2);

    stat = battery_get_stat(BATT_CMD_RELATIVE_STATE_OF_CHARGE);
    if (battery_stat_is_expired(stat)) {
        display_set_text_color(COLOR_GRAY);
        display_print("...");
    } else if (battery_stat_is_error(stat)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
    } else {
        display_set_text_color(COLOR_WHITE);
        display_printf("%d%%", *((uint16_t*) stat->cached_result));
    }

    display_set_text_position(0, 45);
    display_set_text_scale(1);
    display_print("\n");

    stat = battery_get_stat(BATT_CMD_VOLTAGE);
    if (battery_stat_is_expired(stat)) {
        display_set_text_color(COLOR_GRAY);
        display_print("...");
    } else if (battery_stat_is_error(stat)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
    } else {
        display_set_text_color(COLOR_GREEN);
        display_printf("%.2f V", (*((uint16_t*) stat->cached_result)) / 1000.0);
    }

    display_print(" ");

    stat = battery_get_stat(BATT_CMD_CURRENT);
    if (battery_stat_is_expired(stat)) {
        display_set_text_color(COLOR_GRAY);
        display_print("...");
    } else if (battery_stat_is_error(stat)) {
        display_set_text_color(COLOR_RED);
        display_print("error");
    } else {
        display_set_text_color(COLOR_RED);
        display_printf("%.2f A", (*((int16_t*) stat->cached_result)) / 1000.0);
    }

    battery_stat_unlock();


}

void defused_aod_init() {
    display_set_contrast(0);
}


menu_binding_t defused_aod_menu_binding = {
    display_update_interval: 1000000,
    burn_margin_x: 5,
    burn_margin_y: 20,

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
