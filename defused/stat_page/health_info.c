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
g_text_box_t* stat_page_health_verdict_label_text;
g_text_box_t* stat_page_health_verdict_text;

battery_stat_t* stat_page_health_temperature;
battery_stat_t* stat_page_health_full_capacity;
battery_stat_t* stat_page_health_design_capacity;
battery_stat_t* stat_page_health_cycle_count;
battery_stat_t* stat_page_health_manufacture_date;

struct health_verdict {
    char* text;
    color_t color;
};
typedef struct health_verdict health_verdict_t;

health_verdict_t VERDICT_UNKNOWN = {"?", COLOR_GRAY};
health_verdict_t VERDICT_NEW = {"new", COLOR_BLUE};
health_verdict_t VERDICT_GOOD = {"good", COLOR_GREEN};
health_verdict_t VERDICT_OK = {"OK", COLOR_YELLOW};
health_verdict_t VERDICT_BAD = {"BAD", COLOR_ORANGE};
health_verdict_t VERDICT_EOL = {"EOL!", COLOR_RED};

// 0 - 5
health_verdict_t* health_info_verdicts_single_dimensional[] = {
    &VERDICT_UNKNOWN, &VERDICT_NEW, &VERDICT_GOOD, &VERDICT_OK, &VERDICT_BAD, &VERDICT_EOL
};

health_verdict_t VERDICT_CALIBRATE = {"calibrate", COLOR_GRAY};   // unrealistic and suspicious
health_verdict_t VERDICT_LUCK = {"luck?", COLOR_YELLOW};          // unrealistic but not suspicious
health_verdict_t VERDICT_NORMAL = {"normal", COLOR_GREEN};        // aging as expected
health_verdict_t VERDICT_WORN = {"old", COLOR_YELLOW};            // aging as expected but close to EOL
health_verdict_t VERDICT_UNHEALTHY = {"unhealthy", COLOR_RED};    // aging a little faster than normal
health_verdict_t VERDICT_ABUSE = {"damaged", COLOR_RED};          // aging much faster than normal
health_verdict_t VERDICT_SPICY = {"destroyed", COLOR_RED};        // aging way too fast - defective? neglected?

health_verdict_t* health_info_verdicts_cycle_wear_matrix[6][6] = {
            // wear
/* cycles *//*  <-10%               <=10%               <=20%               <=30%               <=40%               >40%                    */
/* ???    */{   &VERDICT_UNKNOWN,   &VERDICT_NEW,       &VERDICT_GOOD,      &VERDICT_OK,        &VERDICT_BAD,       &VERDICT_EOL            },

/* <=300  */{   &VERDICT_CALIBRATE, &VERDICT_NEW,       &VERDICT_UNHEALTHY, &VERDICT_ABUSE,     &VERDICT_SPICY,     &VERDICT_SPICY          },
/* <=500  */{   &VERDICT_CALIBRATE, &VERDICT_GOOD,      &VERDICT_OK,        &VERDICT_UNHEALTHY, &VERDICT_ABUSE,     &VERDICT_SPICY          },
/* <=1000 */{   &VERDICT_CALIBRATE, &VERDICT_LUCK,      &VERDICT_NORMAL,    &VERDICT_OK,        &VERDICT_WORN,      &VERDICT_EOL            },
/* <=1500 */{   &VERDICT_CALIBRATE, &VERDICT_LUCK,      &VERDICT_GOOD,      &VERDICT_NORMAL,    &VERDICT_WORN,      &VERDICT_EOL            },
/* >1500  */{   &VERDICT_CALIBRATE, &VERDICT_CALIBRATE, &VERDICT_LUCK,      &VERDICT_GOOD,      &VERDICT_WORN,      &VERDICT_EOL            },
};

uint health_info_get_wear_verdict_index(float calculated_wear) {
    if (calculated_wear < -10) return 0;          // likely very mis-calibrated
    else if (calculated_wear <= 10) return 1;
    else if (calculated_wear <= 20) return 2;
    else if (calculated_wear <= 30) return 3;
    else if (calculated_wear <= 40) return 4;
    else return 5;
}

uint health_info_get_cycle_verdict_index(uint16_t cycle_count) {
    if (cycle_count <= 300) return 1;
    else if (cycle_count <= 500) return 2;
    else if (cycle_count <= 1000) return 3;
    else if (cycle_count <= 1500) return 4;
    else return 5;
}

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
    stat_page_health_verdict_label_text = get_g_text_box_inst();
    stat_page_health_verdict_text = get_g_text_box_inst();
    

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

    setup_g_text_box(stat_page_health_verdict_label_text, 0, y, label_col_width - 1, 1, COLOR_WHITE);
    g_text_box_print(stat_page_health_verdict_label_text, "verd");
    stat_page_health_verdict_label_text->alignment_mode = TEXT_ALIGN_RIGHT;
    y += g_text_box_height(stat_page_health_verdict_label_text) + line_spacing;
    
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
    
    setup_g_text_box(stat_page_health_verdict_text, value_col_x, y, display_area_width() - 1, 1, 0);
    stat_page_health_verdict_text->truncation_mode = TEXT_MARQUEE;
    y += g_text_box_height(stat_page_health_verdict_text) + line_spacing;
    

    graphics_add_text_box(stat_page_health_capacity_ratio_text);

    graphics_add_text_box(stat_page_health_wear_text);
    graphics_add_text_box(stat_page_health_cycle_count_text);
    graphics_add_text_box(stat_page_health_temperature_text);
    graphics_add_text_box(stat_page_health_verdict_text);

    graphics_add_text_box(stat_page_health_wear_label_text);
    graphics_add_text_box(stat_page_health_cycle_count_label_text);
    graphics_add_text_box(stat_page_health_temperature_label_text);
    graphics_add_text_box(stat_page_health_verdict_label_text);
}

void defused_stat_page_health_info_update() {
    float full_capacity;
    float design_capacity;

    float calculated_wear;
    uint wear_verdict_i = 0;

    uint16_t cycle_count;
    uint cycle_verdict_i = 0;
    
    health_verdict_t* grand_verdict;

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
        wear_verdict_i = health_info_get_wear_verdict_index(calculated_wear);

        stat_page_health_wear_text->color = health_info_verdicts_single_dimensional[wear_verdict_i]->color;
        g_text_box_printf(stat_page_health_wear_text,
            "%.0f%% %s", calculated_wear, health_info_verdicts_single_dimensional[wear_verdict_i]->text);

    }
    
    // cycle count
    if (!defused_print_batt_stat_error(stat_page_health_cycle_count_text, stat_page_health_cycle_count, COLOR_GRAY, "---", "error")) {

        cycle_count = *stat_page_health_cycle_count->cached_result.as_uint16;
        cycle_verdict_i = health_info_get_cycle_verdict_index(cycle_count);
        
        stat_page_health_cycle_count_text->color = health_info_verdicts_single_dimensional[cycle_verdict_i]->color;
        g_text_box_printf(stat_page_health_cycle_count_text,
            "%d %s", cycle_count, health_info_verdicts_single_dimensional[cycle_verdict_i]->text);
    }
    
    // temperature
    if (!defused_print_batt_stat_error(stat_page_health_temperature_text, stat_page_health_temperature, COLOR_BLUE, "---.- K", "error")) {
        g_text_box_printf(stat_page_health_temperature_text, 
            "%.1f K", (*stat_page_health_temperature->cached_result.as_uint16) / 10.0);
    }
    
    // grand verdict (doesn't take temp into account)
    if (!battery_stat_is_valid(stat_page_health_full_capacity) || !battery_stat_is_valid(stat_page_health_design_capacity) || !battery_stat_is_valid(stat_page_health_cycle_count)) {
        grand_verdict = &VERDICT_UNKNOWN;
    } else {
        grand_verdict = health_info_verdicts_cycle_wear_matrix[cycle_verdict_i][wear_verdict_i];
    }

    stat_page_health_verdict_text->color = grand_verdict->color;
    g_text_box_print(stat_page_health_verdict_text, grand_verdict->text);

    battery_stat_unlock();
}