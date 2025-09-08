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
#include "defused/stat_page/cell_voltage_info.h"
#include "defused/batt_gui_util.h"
#include "defused/gui.h"
#include "display.h"

g_text_box_t* stat_page_cell_voltage_label_texts[CELL_VOLTAGE_INFO_CELL_COUNT];
g_text_box_t* stat_page_cell_voltage_value_texts[CELL_VOLTAGE_INFO_CELL_COUNT];

battery_stat_t* stat_page_cell_voltage_mf_data;

void defused_stat_page_cell_voltage_info_init() {
    stat_page_cell_voltage_mf_data = battery_get_stat(BATT_CMD_MANUFACTURER_DATA);
    
    battery_stat_request_update(stat_page_cell_voltage_mf_data);
    
    for (uint i = 0; i < CELL_VOLTAGE_INFO_CELL_COUNT; i++) {
        stat_page_cell_voltage_label_texts[i] = get_g_text_box_inst();
        stat_page_cell_voltage_value_texts[i] = get_g_text_box_inst();
    }
    
    
    // layout
    coord_t line_spacing = 3;
    coord_t y = 0;
    coord_t label_col_width = graphics_calculate_text_width(6, 1);
    coord_t value_col_x = graphics_calculate_text_width(7, 1) + 1;

    for (uint i = 0; i < CELL_VOLTAGE_INFO_CELL_COUNT; i++) {
        setup_g_text_box(stat_page_cell_voltage_label_texts[i], 0, y, label_col_width, 1, COLOR_WHITE);
        stat_page_cell_voltage_label_texts[i]->alignment_mode = TEXT_ALIGN_RIGHT;
        g_text_box_printf(stat_page_cell_voltage_label_texts[i], "cell %d", i);
        
        setup_g_text_box(stat_page_cell_voltage_value_texts[i], value_col_x, y, display_area_width() - 1, 1, 0);
        stat_page_cell_voltage_value_texts[i]->truncation_mode = TEXT_MARQUEE;
        
        y += g_text_box_height(stat_page_cell_voltage_label_texts[i]) + line_spacing;
        
        graphics_add_text_box(stat_page_cell_voltage_label_texts[i]);
        graphics_add_text_box(stat_page_cell_voltage_value_texts[i]);
    }

}

void defused_stat_page_cell_voltage_info_update() {
    battery_stat_request_update(stat_page_cell_voltage_mf_data);
    
#ifdef LENOVO_CELL_VOLTAGES

    battery_stat_lock();
    
    g_text_box_t* value_text;
    uint16_t* mf_data = stat_page_cell_voltage_mf_data->cached_result.as_uint16;
    uint16_t cell_voltage;

    for (uint i = 0; i < CELL_VOLTAGE_INFO_CELL_COUNT; i++) {
        value_text = stat_page_cell_voltage_value_texts[i];
        if (defused_print_batt_stat_error(value_text, stat_page_cell_voltage_mf_data, COLOR_BLUE, "-.--- V", "error")) continue;
        if (stat_page_cell_voltage_mf_data->result_length != 14) {
            value_text->color = COLOR_RED;
            g_text_box_print(value_text, "invalid");
            continue;
        }
        
        cell_voltage = mf_data[2 + i];

        g_text_box_printf(value_text, "%.3f V", cell_voltage / 1000.0);
    }

    battery_stat_unlock();

#endif
}
