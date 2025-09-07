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
#include "defused/batt_gui_util.h"

g_text_box_t* aod_charge_text;
g_text_box_t* aod_voltage_text;
g_text_box_t* aod_current_text;
g_text_box_t* aod_remaining_capacity_text;

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

bool aod_print_stat_error(battery_stat_t* stat, g_text_box_t* text_box) {
    if (battery_stat_is_expired(stat)) {
        text_box->color = COLOR_GRAY;
        g_text_box_print(text_box, "...");
        return true;
    } else if (battery_stat_is_error(stat)) {
        text_box->color = COLOR_RED;
        g_text_box_print(text_box, "error");
        return true;
    }
    return false;
}

void defused_aod_update_display() {


    battery_stat_lock();

    if (!defused_print_batt_stat_error(aod_charge_text, aod_charge, COLOR_WHITE, "--%", "ERR%")) {
        g_text_box_printf(aod_charge_text, 
            "%d%%", *aod_charge->cached_result.as_uint16);
    }
    
    if (!defused_print_batt_stat_error(aod_remaining_capacity_text, aod_remaining_capacity, COLOR_GRAY, "--.-- Wh", "error")) {
        aod_remaining_capacity_text->color = COLOR_GRAY;
        g_text_box_printf(aod_remaining_capacity_text, 
            "%.2f Wh", (*aod_remaining_capacity->cached_result.as_uint16) / 100.0);
    }

    if (!defused_print_batt_stat_error(aod_voltage_text, aod_voltage, COLOR_GREEN, "--.-- V", "error")) {
        g_text_box_printf(aod_voltage_text, 
            "%.2f V", (*aod_voltage->cached_result.as_uint16) / 1000.0);
    }

    if (!defused_print_batt_stat_error(aod_current_text, aod_current, COLOR_RED, "--.-- A", "error")) {
        g_text_box_printf(aod_current_text, 
            "%+.2f A", (*aod_current->cached_result.as_int16) / 1000.0);
    }

    battery_stat_unlock();


    graphics_render();
    display_burn_update(true);

    aod_update_stats();
}

void defused_aod_init() {
    coord_t y;

    aod_charge = battery_get_stat(BATT_CMD_RELATIVE_STATE_OF_CHARGE);
    aod_voltage = battery_get_stat(BATT_CMD_VOLTAGE);
    aod_current = battery_get_stat(BATT_CMD_CURRENT);
    aod_remaining_capacity = battery_get_stat(BATT_CMD_REMAINING_CAPACITY);
    
    aod_update_stats();
    
    graphics_reset();
    
    aod_charge_text = get_g_text_box_inst();
    aod_voltage_text = get_g_text_box_inst();
    aod_current_text = get_g_text_box_inst();
    aod_remaining_capacity_text = get_g_text_box_inst();
    

    // layout

    // charge %. big, centered, 10th of the way down
    y = display_area_height() / 10;
    setup_g_text_box(aod_charge_text, 
        0, y, display_area_width() - 1, 
        1, 0);
    aod_charge_text->scale_factor = 2;
    aod_charge_text->alignment_mode = TEXT_ALIGN_CENTER;
    aod_charge_text->truncation_mode = TEXT_MARQUEE;


    // remaining capacity. right under %
    y += g_text_box_height(aod_charge_text) + 5;
    setup_g_text_box(aod_remaining_capacity_text, 
        0, y, display_area_width() - 1, 
        1, 0);
    aod_remaining_capacity_text->alignment_mode = TEXT_ALIGN_CENTER;
    aod_remaining_capacity_text->truncation_mode = TEXT_MARQUEE;

    // voltage. left side
    setup_g_text_box(aod_voltage_text, 
        0, 0, display_area_width() / 2 - 1, 
        1, 0);
    aod_voltage_text->alignment_mode = TEXT_ALIGN_CENTER;
    aod_voltage_text->truncation_mode = TEXT_MARQUEE;

    // current. right side
    setup_g_text_box(aod_current_text, 
        aod_voltage_text->x2 + 1, 0, 
        display_area_width() - 1, 
        1, 0);
    aod_current_text->alignment_mode = TEXT_ALIGN_CENTER;
    aod_current_text->truncation_mode = TEXT_MARQUEE;

    // slam voltage and current to the bottom
    aod_voltage_text->y1 = display_area_height() - g_text_box_height(aod_voltage_text);
    aod_current_text->y1 = display_area_height() - g_text_box_height(aod_voltage_text);
    

    graphics_add_text_box(aod_charge_text);
    graphics_add_text_box(aod_voltage_text);
    graphics_add_text_box(aod_current_text);
    graphics_add_text_box(aod_remaining_capacity_text);
}


menu_binding_t defused_aod_menu_binding = {
    display_update_interval: 1000000,
    burn_margin_x: 0,
    burn_margin_y: 15,

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
