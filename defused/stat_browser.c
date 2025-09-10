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
#include "graphics.h"
#include "display.h"
#include <stdlib.h>

#include "defused/stat_page/general_info.h"
#include "defused/stat_page/health_info.h"
#include "defused/stat_page/cell_voltage_info.h"
#include "defused/stat_page/manufacture_info.h"

g_text_box_t* stat_browser_page_number_text;
g_text_box_t* stat_browser_page_title_text;
g_rectangle_t* stat_browser_bottom_bar;

struct stat_browser_page {
    char* title;
    void (*update_display)();
    void (*init)();
};
typedef struct stat_browser_page stat_browser_page_t;

size_t stat_browser_page_index = 0;
stat_browser_page_t stat_browser_page_defs[] = {
    (stat_browser_page_t){
        title: "general",
        update_display: &defused_stat_page_general_info_update,
        init: &defused_stat_page_general_info_init,
    },
    (stat_browser_page_t){
        title: "health",
        update_display: &defused_stat_page_health_info_update,
        init: &defused_stat_page_health_info_init,
    },
#ifdef CELL_VOLTAGE_INFO_AVAILABLE 
    (stat_browser_page_t){
        title: "cell voltage",
        update_display: &defused_stat_page_cell_voltage_info_update,
        init: &defused_stat_page_cell_voltage_info_init,
    },
#endif
    (stat_browser_page_t){
        title: "manufacture info",
        update_display: &defused_stat_page_manufacture_info_update,
        init: &defused_stat_page_manufacture_info_init,
    },
};


bool defused_stat_browser_on_button_event(button_func_t function, button_event_t event) { return false; }

void defused_stat_browser_on_nav_up() {
    if (stat_browser_page_index == 0) return;
    stat_browser_page_index--;
    bind_stat_browser();    // re-bind
}

void defused_stat_browser_on_nav_down() {
    if (stat_browser_page_index >= sizeof(stat_browser_page_defs) / sizeof(stat_browser_page_t) - 1) return;
    stat_browser_page_index++;
    bind_stat_browser();    // re-bind
}

void defused_stat_browser_on_pre_select() {}
void defused_stat_browser_on_cancel_select() {}
void defused_stat_browser_on_select() {}

bool defused_stat_browser_on_select_held() {
    defused_enter_inactive_mode();
    return true;
}


void defused_stat_browser_update_display() {
    stat_browser_page_t* page = &stat_browser_page_defs[stat_browser_page_index];

    g_text_box_printf(stat_browser_page_number_text, "%d", stat_browser_page_index);
    g_text_box_print(stat_browser_page_title_text, page->title);
    
    page->update_display();
    
    graphics_render();
    display_burn_update(true);
}

void defused_stat_browser_init() {
    
    graphics_reset();
    
    stat_browser_bottom_bar = get_g_rectangle_inst();
    stat_browser_page_title_text = get_g_text_box_inst();
    stat_browser_page_number_text = get_g_text_box_inst();
    

    // layout
    coord_t bottom_bar_padding = 1;

    // page number
    setup_g_text_box(stat_browser_page_number_text, 
        display_area_width() - 1 - graphics_calculate_text_width(3, 1) - bottom_bar_padding, 
        display_area_height() - 1 - bottom_bar_padding,
        display_area_width() - 1 - bottom_bar_padding, 
        1, COLOR_WHITE);
    stat_browser_page_number_text->y1 -= g_text_box_height(stat_browser_page_number_text);
    stat_browser_page_number_text->alignment_mode = TEXT_ALIGN_RIGHT;
    stat_browser_page_number_text->truncation_mode = TEXT_MARQUEE;

    // page title
    setup_g_text_box(stat_browser_page_title_text, 
        bottom_bar_padding,
        stat_browser_page_number_text->y1,
        stat_browser_page_number_text->x2 - 2, 
        1, COLOR_WHITE);
    stat_browser_page_title_text->truncation_mode = TEXT_MARQUEE;
    
    // bottom bar background
    setup_g_rectangle(stat_browser_bottom_bar, 
        0, display_area_height() - g_text_box_height(stat_browser_page_title_text) - bottom_bar_padding * 2, 
        display_area_width() - 1,
        display_area_height() - 1, 
        COLOR_FAINT_BLUE, true);


    stat_browser_page_defs[stat_browser_page_index].init();


    graphics_add_rectangle(stat_browser_bottom_bar);
    graphics_add_text_box(stat_browser_page_title_text);
    graphics_add_text_box(stat_browser_page_number_text);
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

void bind_stat_browser() {
    defused_bind(&defused_stat_browser_menu_binding);
}
