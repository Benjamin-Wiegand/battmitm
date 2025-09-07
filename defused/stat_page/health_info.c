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
#include "defused/stat_page/health_info.h"
#include "defused/batt_gui_util.h"
#include "defused/gui.h"
#include "display.h"

g_text_box_t* stat_page_health_capacity_ratio_text;
g_text_box_t* stat_page_health_wear_label_text;
g_text_box_t* stat_page_health_wear_text;
g_text_box_t* stat_page_health_cycle_count_label_text;
g_text_box_t* stat_page_health_cycle_count_text;
g_text_box_t* stat_page_health_temperature_label_text;
g_text_box_t* stat_page_health_temperature_text;

battery_stat_t* stat_page_health_temperature;
battery_stat_t* stat_page_health_full_capacity;
battery_stat_t* stat_page_health_design_capacity;
battery_stat_t* stat_page_health_cycle_count;
battery_stat_t* stat_page_health_manufacture_date;


void defused_stat_page_health_info_init() {
    stat_page_health_temperature = battery_get_stat(BATT_CMD_TEMPERATURE);
    stat_page_health_full_capacity = battery_get_stat(BATT_CMD_FULL_CHARGE_CAPACITY);
    stat_page_health_design_capacity = battery_get_stat(BATT_CMD_DESIGN_CAPACITY);
    stat_page_health_cycle_count = battery_get_stat(BATT_CMD_CYCLE_COUNT);
    stat_page_health_manufacture_date = battery_get_stat(BATT_CMD_MANUFACTURE_DATE);

    battery_stat_request_update(stat_page_health_temperature);
    battery_stat_request_update(stat_page_health_full_capacity);
    battery_stat_request_update(stat_page_health_design_capacity);
    battery_stat_request_update(stat_page_health_cycle_count);
    battery_stat_request_update(stat_page_health_manufacture_date); //TODO

    stat_page_health_capacity_ratio_text = get_g_text_box_inst();
    stat_page_health_wear_label_text = get_g_text_box_inst();
    stat_page_health_wear_text = get_g_text_box_inst();
    stat_page_health_cycle_count_label_text = get_g_text_box_inst();
    stat_page_health_cycle_count_text = get_g_text_box_inst();
    stat_page_health_temperature_label_text = get_g_text_box_inst();
    stat_page_health_temperature_text = get_g_text_box_inst();
    

    // layout
    coord_t line_spacing = 3;
    coord_t label_col_width = graphics_calculate_text_width(5, 1);
    coord_t value_col_x = graphics_calculate_text_width(6, 1) + 1;
    coord_t y = 0;

    // capacity ratio. right, full width
    setup_g_text_box(stat_page_health_capacity_ratio_text, 0, y, display_area_width() - 1, 1, 0);
    stat_page_health_capacity_ratio_text->alignment_mode = TEXT_ALIGN_RIGHT;
    stat_page_health_capacity_ratio_text->truncation_mode = TEXT_MARQUEE;
    
    y += g_text_box_height(stat_page_health_capacity_ratio_text) + line_spacing;
    
    // labels
    setup_g_text_box(stat_page_health_wear_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_health_wear_label_text, "wear");
    stat_page_health_wear_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_health_wear_label_text) + line_spacing;

    setup_g_text_box(stat_page_health_cycle_count_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_health_cycle_count_label_text, "cycle");
    stat_page_health_cycle_count_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_health_cycle_count_label_text) + line_spacing;

    setup_g_text_box(stat_page_health_temperature_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_health_temperature_label_text, "temp");
    stat_page_health_temperature_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_health_temperature_label_text) + line_spacing;
    
    // back to top
    y = stat_page_health_wear_label_text->y1;

    // values
    setup_g_text_box(stat_page_health_wear_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_health_wear_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_health_wear_text) + line_spacing;

    setup_g_text_box(stat_page_health_cycle_count_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_health_cycle_count_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_health_cycle_count_text) + line_spacing;

    setup_g_text_box(stat_page_health_temperature_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_health_temperature_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_health_temperature_text) + line_spacing;
    

    graphics_add_text_box(stat_page_health_capacity_ratio_text);

    graphics_add_text_box(stat_page_health_wear_text);
    graphics_add_text_box(stat_page_health_cycle_count_text);
    graphics_add_text_box(stat_page_health_temperature_text);

    graphics_add_text_box(stat_page_health_wear_label_text);
    graphics_add_text_box(stat_page_health_cycle_count_label_text);
    graphics_add_text_box(stat_page_health_temperature_label_text);
}

void defused_stat_page_health_info_update() {
    float full_capacity;
    float design_capacity;
    float calculated_wear;
    char* wear_verdict;
    uint16_t cycle_count;
    char* cycle_verdict;
    battery_stat_request_update(stat_page_health_temperature);
    battery_stat_request_update(stat_page_health_full_capacity);
    battery_stat_request_update(stat_page_health_design_capacity);
    battery_stat_request_update(stat_page_health_cycle_count);
    battery_stat_request_update(stat_page_health_manufacture_date); //TODO


    battery_stat_lock();

    // capacity ratio + health
    if (battery_stat_is_expired(stat_page_health_full_capacity) || battery_stat_is_expired(stat_page_health_design_capacity)) {
        stat_page_health_capacity_ratio_text->color = COLOR_GRAY;
        g_text_box_print(stat_page_health_capacity_ratio_text, "--.-/--.- Wh");
        stat_page_health_wear_text->color = COLOR_GRAY;
        g_text_box_print(stat_page_health_wear_text, "--.-%");
    } else if (battery_stat_is_error(stat_page_health_full_capacity) || battery_stat_is_error(stat_page_health_design_capacity)) {
        stat_page_health_capacity_ratio_text->color = COLOR_RED;
        g_text_box_print(stat_page_health_capacity_ratio_text, "error");
        stat_page_health_wear_text->color = COLOR_RED;
        g_text_box_print(stat_page_health_wear_text, "error");
    } else {
        
        // capacity ratio
        full_capacity = (*stat_page_health_full_capacity->cached_result.as_uint16) / 100.0;
        design_capacity = (*stat_page_health_design_capacity->cached_result.as_uint16) / 100.0;
        stat_page_health_capacity_ratio_text->color = COLOR_GRAY;
        g_text_box_printf(stat_page_health_capacity_ratio_text,
            "%.1f/%.1f Wh", full_capacity, design_capacity);
        
        // wear %
        calculated_wear = 100 - (100 * full_capacity / design_capacity);
        if (calculated_wear < 0) {
            // TODO: should probably allow getting values from overrides for things like this
            stat_page_health_wear_text->color = COLOR_GRAY;
            wear_verdict = "new?";  // either extremely new, modified (like mine), freshly re-celled, or mis-calibrated
        } else if (calculated_wear <= 10) {
            stat_page_health_wear_text->color = COLOR_BLUE;
            wear_verdict = "new";
        } else if (calculated_wear <= 20) {
            stat_page_health_wear_text->color = COLOR_GREEN;
            wear_verdict = "good";
        } else if (calculated_wear <= 30) {
            stat_page_health_wear_text->color = COLOR_YELLOW;
            wear_verdict = "OK";
        } else if (calculated_wear <= 40) {
            stat_page_health_wear_text->color = COLOR_ORANGE;
            wear_verdict = "BAD";
        } else {
            stat_page_health_wear_text->color = COLOR_RED;
            wear_verdict = "EOL!";
        }

        g_text_box_printf(stat_page_health_wear_text,
            "%.0f%% %s", calculated_wear, wear_verdict);

    }
    
    // cycle count
    if (!defused_print_batt_stat_error(stat_page_health_cycle_count_text, stat_page_health_cycle_count, COLOR_GRAY, "---", "error")) {
        cycle_count = *stat_page_health_cycle_count->cached_result.as_uint16;
        if (cycle_count <= 100) {
            stat_page_health_cycle_count_text->color = COLOR_BLUE;
            cycle_verdict = "new";
        } else if (cycle_count <= 400) {
            stat_page_health_cycle_count_text->color = COLOR_GREEN;
            cycle_verdict = "good";
        } else if (cycle_count <= 1000) {
            stat_page_health_cycle_count_text->color = COLOR_YELLOW;
            cycle_verdict = "OK";
        } else if (cycle_count <= 1500) {
            stat_page_health_cycle_count_text->color = COLOR_ORANGE;
            cycle_verdict = "BAD";
        } else {
            stat_page_health_cycle_count_text->color = COLOR_RED;
            cycle_verdict = "EOL!";
        }
        g_text_box_printf(stat_page_health_cycle_count_text,
            "%d %s", cycle_count, cycle_verdict);
    }
    
    // temperature
    if (!defused_print_batt_stat_error(stat_page_health_temperature_text, stat_page_health_temperature, COLOR_BLUE, "---.- K", "error")) {
        g_text_box_printf(stat_page_health_temperature_text, 
            "%.1f K", (*stat_page_health_temperature->cached_result.as_uint16) / 10.0);
    }

    battery_stat_unlock();
}