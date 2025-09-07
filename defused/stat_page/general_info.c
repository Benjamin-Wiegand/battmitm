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
#include "defused/stat_page/general_info.h"
#include "defused/batt_gui_util.h"
#include "defused/gui.h"
#include "display.h"

g_text_box_t* stat_page_general_charge_text;
g_text_box_t* stat_page_general_max_error_text;
g_text_box_t* stat_page_general_remaining_capacity_text;
g_line_t* stat_page_general_divider_line;
g_text_box_t* stat_page_general_voltage_text;
g_text_box_t* stat_page_general_current_text;
g_text_box_t* stat_page_general_temperature_text;
g_text_box_t* stat_page_general_wattage_text;

battery_stat_t* stat_page_general_charge;
battery_stat_t* stat_page_general_max_error;
battery_stat_t* stat_page_general_voltage;
battery_stat_t* stat_page_general_current;
battery_stat_t* stat_page_general_temperature;
battery_stat_t* stat_page_general_remaining_capacity;


void defused_stat_page_general_info_init() {
    stat_page_general_charge = battery_get_stat(BATT_CMD_RELATIVE_STATE_OF_CHARGE);
    stat_page_general_max_error = battery_get_stat(BATT_CMD_MAX_ERROR);
    stat_page_general_voltage = battery_get_stat(BATT_CMD_VOLTAGE);
    stat_page_general_current = battery_get_stat(BATT_CMD_CURRENT);
    stat_page_general_temperature = battery_get_stat(BATT_CMD_TEMPERATURE);
    stat_page_general_remaining_capacity = battery_get_stat(BATT_CMD_REMAINING_CAPACITY);

    battery_stat_request_update(stat_page_general_charge);
    battery_stat_request_update(stat_page_general_max_error);
    battery_stat_request_update(stat_page_general_voltage);
    battery_stat_request_update(stat_page_general_current);
    battery_stat_request_update(stat_page_general_temperature);
    battery_stat_request_update(stat_page_general_remaining_capacity);
    
    stat_page_general_charge_text = get_g_text_box_inst();
    stat_page_general_max_error_text = get_g_text_box_inst();
    stat_page_general_remaining_capacity_text = get_g_text_box_inst();
    stat_page_general_divider_line = get_g_line_inst();
    stat_page_general_voltage_text = get_g_text_box_inst();
    stat_page_general_current_text = get_g_text_box_inst();
    stat_page_general_temperature_text = get_g_text_box_inst();
    stat_page_general_wattage_text = get_g_text_box_inst();
    

    // layout
    coord_t line_spacing = 3;
    coord_t y = 0;

    // charge %
    setup_g_text_box(stat_page_general_charge_text, 
        0, y, graphics_calculate_text_width(4, 2), 1, 0);
    stat_page_general_charge_text->scale_factor = 2;
    stat_page_general_charge_text->truncation_mode = TEXT_MARQUEE;
    
    // charge % error. to bottom right of %
    setup_g_text_box(stat_page_general_max_error_text,
        stat_page_general_charge_text->x2 + 6, y,
        display_area_width() - 1, 
        1, COLOR_GRAY);
    stat_page_general_max_error_text->y1 += g_text_box_height(stat_page_general_charge_text) - g_text_box_height(stat_page_general_max_error_text);
    stat_page_general_max_error_text->truncation_mode = TEXT_MARQUEE;

    y += g_text_box_height(stat_page_general_charge_text) + line_spacing;

    // remaining capacity. below %
    setup_g_text_box(stat_page_general_remaining_capacity_text,
        0, y, display_area_width() - 1, 1, 0);
    stat_page_general_remaining_capacity_text->alignment_mode = TEXT_ALIGN_LEFT;
    stat_page_general_remaining_capacity_text->truncation_mode = TEXT_MARQUEE;
    
    y += g_text_box_height(stat_page_general_remaining_capacity_text) + line_spacing;

    // divider
    setup_g_line(stat_page_general_divider_line, 0, y, display_area_width() - 1, y, COLOR_GRAY);

    y += line_spacing;

    // voltage. left in grid
    setup_g_text_box(stat_page_general_voltage_text,
        0, y, display_area_width() / 2 - 1, 1, 0);
    stat_page_general_voltage_text->alignment_mode = TEXT_ALIGN_RIGHT;
    stat_page_general_voltage_text->truncation_mode = TEXT_MARQUEE;
    
    // current. right in grid
    setup_g_text_box(stat_page_general_current_text,
        stat_page_general_voltage_text->x2 + 1, y,
        display_area_width() - 1,
        1, 0);
    stat_page_general_current_text->alignment_mode = TEXT_ALIGN_RIGHT;
    stat_page_general_current_text->truncation_mode = TEXT_MARQUEE;
    
    y += g_text_box_height(stat_page_general_voltage_text) + line_spacing;

    // temperature. left in grid
    setup_g_text_box(stat_page_general_temperature_text,
        0, y, display_area_width() / 2 - 1, 1, 0);
    stat_page_general_temperature_text->alignment_mode = TEXT_ALIGN_RIGHT;
    stat_page_general_temperature_text->truncation_mode = TEXT_MARQUEE;
    
    // wattage. right in grid
    setup_g_text_box(stat_page_general_wattage_text,
        stat_page_general_temperature_text->x2 + 1, y,
        display_area_width() - 1,
        1, COLOR_GRAY);
    stat_page_general_wattage_text->alignment_mode = TEXT_ALIGN_RIGHT;
    stat_page_general_wattage_text->truncation_mode = TEXT_MARQUEE;

    graphics_add_text_box(stat_page_general_charge_text);
    graphics_add_text_box(stat_page_general_max_error_text);
    graphics_add_text_box(stat_page_general_remaining_capacity_text);
    graphics_add_line(stat_page_general_divider_line);
    graphics_add_text_box(stat_page_general_voltage_text);
    graphics_add_text_box(stat_page_general_current_text);
    graphics_add_text_box(stat_page_general_temperature_text);
    graphics_add_text_box(stat_page_general_wattage_text);
}

void defused_stat_page_general_info_update() {
    uint16_t max_error;
    float voltage;
    float current;
    battery_stat_request_update(stat_page_general_charge);
    battery_stat_request_update(stat_page_general_max_error);
    battery_stat_request_update(stat_page_general_voltage);
    battery_stat_request_update(stat_page_general_current);
    battery_stat_request_update(stat_page_general_temperature);
    battery_stat_request_update(stat_page_general_remaining_capacity);
    
    
    battery_stat_lock();

    if (battery_stat_is_valid(stat_page_general_max_error)) {
        max_error = *stat_page_general_max_error->cached_result.as_uint16;
    } else {
        max_error = 0;
    }

    // battery % error
    if (max_error == 0) {
        stat_page_general_max_error_text->enabled = false;
    } else {
        stat_page_general_max_error_text->enabled = true;
        g_text_box_printf(stat_page_general_max_error_text, "+/-%d%%", max_error);
    }

    // battery %
    if (!defused_print_batt_stat_error(stat_page_general_charge_text, stat_page_general_charge, COLOR_WHITE, "--%", "ERR%")) {
        g_text_box_printf(stat_page_general_charge_text, 
            "%d%%", (*stat_page_general_charge->cached_result.as_uint16) + (max_error / 2));
    }

    // battery remaining capacity
    if (!defused_print_batt_stat_error(stat_page_general_remaining_capacity_text, stat_page_general_remaining_capacity, COLOR_GRAY, "--.-- Wh", "error")) {
        g_text_box_printf(stat_page_general_remaining_capacity_text, 
            "%.2f Wh", (*stat_page_general_remaining_capacity->cached_result.as_uint16) / 100.0);
    }

    // battery voltage
    if (!defused_print_batt_stat_error(stat_page_general_voltage_text, stat_page_general_voltage, COLOR_GREEN, "--.-- V", "error")) {
        voltage = (*stat_page_general_voltage->cached_result.as_uint16) / 1000.0;
        g_text_box_printf(stat_page_general_voltage_text, "%.2f V", voltage);
    }
    
    // battery current
    if (!defused_print_batt_stat_error(stat_page_general_current_text, stat_page_general_current, COLOR_RED, "--.-- A", "error")) {
        current = (*stat_page_general_current->cached_result.as_int16) / 1000.0;
        g_text_box_printf(stat_page_general_current_text, "%+.2f A", current);
    }
    
    // battery temperature
    if (!defused_print_batt_stat_error(stat_page_general_temperature_text, stat_page_general_temperature, COLOR_BLUE, "---.- K", "error")) {
        g_text_box_printf(stat_page_general_temperature_text, 
            "%.1f K", (*stat_page_general_temperature->cached_result.as_uint16) / 10.0);
    }
    
    // wattage
    if (!battery_stat_is_valid(stat_page_general_voltage) || !battery_stat_is_valid(stat_page_general_current)) {
        g_text_box_printf(stat_page_general_wattage_text, "--.- W");
    } else {
        g_text_box_printf(stat_page_general_wattage_text, 
            "%+.1f W", voltage * current);
    }

    battery_stat_unlock();
}