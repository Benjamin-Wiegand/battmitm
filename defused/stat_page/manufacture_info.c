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

g_text_box_t* stat_page_manufacture_mf_name_label_text;
g_text_box_t* stat_page_manufacture_mf_name_text;
g_text_box_t* stat_page_manufacture_dev_name_label_text;
g_text_box_t* stat_page_manufacture_dev_name_text;
g_text_box_t* stat_page_manufacture_date_label_text;
g_text_box_t* stat_page_manufacture_date_text;
g_text_box_t* stat_page_manufacture_chemistry_label_text;
g_text_box_t* stat_page_manufacture_chemistry_text;
g_text_box_t* stat_page_manufacture_serial_label_text;
g_text_box_t* stat_page_manufacture_serial_text;

battery_stat_t* stat_page_manufacture_mf_name;
battery_stat_t* stat_page_manufacture_dev_name;
battery_stat_t* stat_page_manufacture_date;
battery_stat_t* stat_page_manufacture_chemistry;
battery_stat_t* stat_page_manufacture_serial;


void defused_stat_page_manufacture_info_init() {

    stat_page_manufacture_mf_name = battery_get_stat(BATT_CMD_MANUFACTURER_NAME);
    stat_page_manufacture_dev_name = battery_get_stat(BATT_CMD_DEVICE_NAME);
    stat_page_manufacture_date = battery_get_stat(BATT_CMD_MANUFACTURE_DATE);
    stat_page_manufacture_chemistry = battery_get_stat(BATT_CMD_DEVICE_CHEMISTRY);
    stat_page_manufacture_serial = battery_get_stat(BATT_CMD_SERIAL_NUMBER);

    battery_stat_request_update(stat_page_manufacture_mf_name);
    battery_stat_request_update(stat_page_manufacture_dev_name);
    battery_stat_request_update(stat_page_manufacture_date);
    battery_stat_request_update(stat_page_manufacture_chemistry);
    battery_stat_request_update(stat_page_manufacture_serial);


    stat_page_manufacture_mf_name_label_text = get_g_text_box_inst();
    stat_page_manufacture_mf_name_text = get_g_text_box_inst();
    stat_page_manufacture_dev_name_label_text = get_g_text_box_inst();
    stat_page_manufacture_dev_name_text = get_g_text_box_inst();
    stat_page_manufacture_date_label_text = get_g_text_box_inst();
    stat_page_manufacture_date_text = get_g_text_box_inst();
    stat_page_manufacture_chemistry_label_text = get_g_text_box_inst();
    stat_page_manufacture_chemistry_text = get_g_text_box_inst();
    stat_page_manufacture_serial_label_text = get_g_text_box_inst();
    stat_page_manufacture_serial_text = get_g_text_box_inst();
    

    // layout
    coord_t line_spacing = 3;
    coord_t label_col_width = graphics_calculate_text_width(5, 1);
    coord_t value_col_x = graphics_calculate_text_width(6, 1) + 1;
    coord_t y = 0;

    // labels
    setup_g_text_box(stat_page_manufacture_mf_name_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_manufacture_mf_name_label_text, "vend");
    stat_page_manufacture_mf_name_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_manufacture_mf_name_label_text) + line_spacing;

    setup_g_text_box(stat_page_manufacture_dev_name_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_manufacture_dev_name_label_text, "dev");
    stat_page_manufacture_dev_name_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_manufacture_dev_name_label_text) + line_spacing;

    setup_g_text_box(stat_page_manufacture_serial_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_manufacture_serial_label_text, "srln");
    stat_page_manufacture_serial_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_manufacture_serial_label_text) + line_spacing;

    setup_g_text_box(stat_page_manufacture_chemistry_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_manufacture_chemistry_label_text, "chem");
    stat_page_manufacture_chemistry_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_manufacture_chemistry_label_text) + line_spacing;

    setup_g_text_box(stat_page_manufacture_date_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_manufacture_date_label_text, "date");
    stat_page_manufacture_date_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_manufacture_date_label_text) + line_spacing;

    
    // back to top
    y = stat_page_manufacture_mf_name_label_text->y1;

    // values
    setup_g_text_box(stat_page_manufacture_mf_name_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_manufacture_mf_name_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_manufacture_mf_name_text) + line_spacing;

    setup_g_text_box(stat_page_manufacture_dev_name_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_manufacture_dev_name_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_manufacture_dev_name_text) + line_spacing;

    setup_g_text_box(stat_page_manufacture_serial_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_manufacture_serial_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_manufacture_serial_text) + line_spacing;

    setup_g_text_box(stat_page_manufacture_chemistry_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_manufacture_chemistry_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_manufacture_chemistry_text) + line_spacing;
    
    setup_g_text_box(stat_page_manufacture_date_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_manufacture_date_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_manufacture_date_text) + line_spacing;
    

    graphics_add_text_box(stat_page_manufacture_mf_name_text);
    graphics_add_text_box(stat_page_manufacture_dev_name_text);
    graphics_add_text_box(stat_page_manufacture_date_text);
    graphics_add_text_box(stat_page_manufacture_chemistry_text);
    graphics_add_text_box(stat_page_manufacture_serial_text);

    graphics_add_text_box(stat_page_manufacture_mf_name_label_text);
    graphics_add_text_box(stat_page_manufacture_dev_name_label_text);
    graphics_add_text_box(stat_page_manufacture_date_label_text);
    graphics_add_text_box(stat_page_manufacture_chemistry_label_text);
    graphics_add_text_box(stat_page_manufacture_serial_label_text);
}

void defused_stat_page_manufacture_info_update() {
    uint16_t date;

    battery_stat_request_update(stat_page_manufacture_mf_name);
    battery_stat_request_update(stat_page_manufacture_dev_name);
    battery_stat_request_update(stat_page_manufacture_date);
    battery_stat_request_update(stat_page_manufacture_chemistry);
    battery_stat_request_update(stat_page_manufacture_serial);


    battery_stat_lock();

    // manufacturer name
    if (!defused_print_batt_stat_error(stat_page_manufacture_mf_name_text, stat_page_manufacture_mf_name, COLOR_GRAY, "...", "error")) {
        g_text_box_printf(stat_page_manufacture_mf_name_text,
            "%.*s", stat_page_manufacture_mf_name->result_length, stat_page_manufacture_mf_name->cached_result.as_uint8);
    }

    // device name
    if (!defused_print_batt_stat_error(stat_page_manufacture_dev_name_text, stat_page_manufacture_dev_name, COLOR_GRAY, "...", "error")) {
        g_text_box_printf(stat_page_manufacture_dev_name_text,
            "%.*s", stat_page_manufacture_dev_name->result_length, stat_page_manufacture_dev_name->cached_result.as_uint8);
    }
    
    // serial number
    if (!defused_print_batt_stat_error(stat_page_manufacture_serial_text, stat_page_manufacture_serial, COLOR_GRAY, "---.- K", "error")) {
        g_text_box_printf(stat_page_manufacture_serial_text, 
            "%d", *stat_page_manufacture_serial->cached_result.as_uint16);
    }

    // device chemistry
    if (!defused_print_batt_stat_error(stat_page_manufacture_chemistry_text, stat_page_manufacture_chemistry, COLOR_GRAY, "...", "error")) {
        g_text_box_printf(stat_page_manufacture_chemistry_text,
            "%.*s", stat_page_manufacture_chemistry->result_length, stat_page_manufacture_chemistry->cached_result.as_uint8);
    }

    // manufacture date
    if (!defused_print_batt_stat_error(stat_page_manufacture_date_text, stat_page_manufacture_date, COLOR_GRAY, "...", "error")) {
        date = *stat_page_manufacture_date->cached_result.as_uint16;
        g_text_box_printf(stat_page_manufacture_date_text,
            "%d/%02d/%02d", 1980 + (date >> 9), (date >> 5) & 0x0F, date & 0x1F);
    }
    
    battery_stat_unlock();
}
