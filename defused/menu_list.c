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
#include "defused/menu_list.h"
#include "graphics.h"
#include "display.h"
#include "font.h"
#include <stdlib.h>

#define MENU_LIST_TITLE_PADDING             1
#define MENU_LIST_ITEM_PADDING              1
#define MENU_LIST_MIN_VERTICAL_BURN_MARGIN  5
#define MENU_LIST_HORIZONTAL_BURN_MARGIN    5
#define MENU_LIST_SCROLL_BAR_WIDTH          2
#define MENU_LIST_SCROLL_BAR_FILL_COLOR     COLOR_GRAY
#define MENU_LIST_SELECTION_HIGHLIGHT_COLOR COLOR_WHITE

#define MENU_LIST_TITLE_HEIGHT              (FONT_HEIGHT + MENU_LIST_TITLE_PADDING * 2)
#define MENU_LIST_START_Y                   MENU_LIST_TITLE_HEIGHT
#define MENU_LIST_ITEM_HEIGHT               (FONT_HEIGHT + MENU_LIST_ITEM_PADDING * 2)
#define MENU_LIST_ONSCREEN_ITEMS            ((DISPLAY_RESOLUTION_HEIGHT - MENU_LIST_START_Y - MENU_LIST_MIN_VERTICAL_BURN_MARGIN) / MENU_LIST_ITEM_HEIGHT)
#define MENU_LIST_SCREEN_Y_LIMIT            (MENU_LIST_ONSCREEN_ITEMS * MENU_LIST_ITEM_HEIGHT + MENU_LIST_START_Y)
#define MENU_LIST_VERTICAL_BURN_MARGIN      (DISPLAY_RESOLUTION_HEIGHT - MENU_LIST_SCREEN_Y_LIMIT)

#define MENU_LIST_TREE_MAX_DEPTH            32

g_text_box_t* menu_list_title_text;
g_rectangle_t* menu_list_title_bg;
g_rectangle_t* menu_list_selected_item_highlight;
g_rectangle_t* menu_list_scroll_bar_fill;

g_text_box_t* menu_list_item_texts[MENU_LIST_ONSCREEN_ITEMS];

menu_list_def_t* menu_list_definition;

size_t menu_list_selected_item_index;
size_t menu_list_item_scroll_index;

menu_list_def_t* menu_list_tree_back_stack[MENU_LIST_TREE_MAX_DEPTH];
size_t menu_list_tree_back_stack_index = 0;

bool menu_list_pre_select = false;


bool defused_menu_list_on_button_event(button_func_t function, button_event_t event) { return false; }

void defused_menu_list_on_nav_up() {
    if (menu_list_selected_item_index <= 0) return;
    menu_list_selected_item_index--;
    defused_update_display_now();
}

void defused_menu_list_on_nav_down() {
    if (menu_list_selected_item_index >= menu_list_definition->items_size - 1) return;
    menu_list_selected_item_index++;
    defused_update_display_now();
}

void defused_menu_list_on_pre_select() {
    menu_list_pre_select = true;
    defused_update_display_now();
}

void defused_menu_list_on_cancel_select() {
    menu_list_pre_select = false;
    defused_update_display_now();
}

void defused_menu_list_on_select() {
    menu_list_pre_select = false;
    menu_list_item_t* selected = &menu_list_definition->items[menu_list_selected_item_index];
    
    switch (selected->action_type) {
        case MENU_LIST_CALLBACK:
            selected->action.callback();
            break;
        case MENU_LIST_TREE:
            if (menu_list_tree_back_stack_index >= MENU_LIST_TREE_MAX_DEPTH - 1) {
                // make it obvious that something broke, without causing problems
                return;
            }

            menu_list_tree_back_stack[menu_list_tree_back_stack_index++] = menu_list_definition;
            menu_list_definition = selected->action.tree;
            bind_menu_list();
            break;

        case MENU_LIST_TREE_BACK:
            if (menu_list_tree_back_stack_index == 0) {
                // nothing to pop off the stack, this shouldn't happen. dump to aod
                defused_enter_inactive_mode();
                return;
            }

            menu_list_definition = menu_list_tree_back_stack[--menu_list_tree_back_stack_index];
            bind_menu_list();
            break;

        default:
            break;
    }
}

bool defused_menu_list_on_select_held() { return false; }

void menu_list_scroll_selection_onscreen() {
    if (menu_list_selected_item_index > menu_list_item_scroll_index + MENU_LIST_ONSCREEN_ITEMS - 1) {
        menu_list_item_scroll_index = menu_list_selected_item_index - MENU_LIST_ONSCREEN_ITEMS + 1;
    } else if (menu_list_selected_item_index < menu_list_item_scroll_index) {
        menu_list_item_scroll_index = menu_list_selected_item_index;
    }
}

void defused_menu_list_update_display() {
    
    // scroll bar
    menu_list_scroll_selection_onscreen();

    size_t max_scroll;
    coord_t scroll_bar_height;
    coord_t scroll_bar_y;
    menu_list_scroll_bar_fill->enabled = menu_list_definition->items_size > MENU_LIST_ONSCREEN_ITEMS;

    if (menu_list_scroll_bar_fill->enabled) {
        scroll_bar_height = (MENU_LIST_ONSCREEN_ITEMS * (MENU_LIST_SCREEN_Y_LIMIT - MENU_LIST_START_Y)) / menu_list_definition->items_size;
        max_scroll = menu_list_definition->items_size - MENU_LIST_ONSCREEN_ITEMS;
        
        scroll_bar_y = menu_list_item_scroll_index * ((MENU_LIST_SCREEN_Y_LIMIT - MENU_LIST_START_Y) - scroll_bar_height) / max_scroll + MENU_LIST_START_Y;

        menu_list_scroll_bar_fill->y1 = scroll_bar_y;
        menu_list_scroll_bar_fill->y2 = scroll_bar_y + scroll_bar_height - 1;
    }
    
    
    // on-screen list entries
    g_text_box_t* text_box;
    menu_list_item_t* menu_item;
    size_t item_index;
    menu_list_selected_item_highlight->enabled = false;

    for (size_t i = 0; i < MENU_LIST_ONSCREEN_ITEMS; i++) {
        text_box = menu_list_item_texts[i];
        item_index = menu_list_item_scroll_index + i;

        text_box->enabled = item_index < menu_list_definition->items_size;
        if (!text_box->enabled) continue;
        
        menu_item = &menu_list_definition->items[item_index];

        if (item_index == menu_list_selected_item_index) {
            text_box->color = menu_list_pre_select ? COLOR_WHITE : COLOR_BLACK;
            menu_list_selected_item_highlight->enabled = true;
            menu_list_selected_item_highlight->filled = !menu_list_pre_select;
            menu_list_selected_item_highlight->y1 = text_box->y1 - MENU_LIST_ITEM_PADDING;
            menu_list_selected_item_highlight->y2 = text_box->y1 + MENU_LIST_ITEM_HEIGHT - 2;
        } else {
            text_box->color = COLOR_WHITE;
        }
        
        g_text_box_print(text_box, menu_item->text);
        
    }

    graphics_render();
    display_burn_update(true);
}


void defused_menu_list_init() {
    g_text_box_t* text_box;

    menu_list_selected_item_index = 0;
    menu_list_item_scroll_index = 0;
    menu_list_pre_select = false;

    graphics_reset();

    menu_list_title_bg = get_g_rectangle_inst();
    menu_list_title_text = get_g_text_box_inst();
    menu_list_selected_item_highlight = get_g_rectangle_inst();
    menu_list_scroll_bar_fill = get_g_rectangle_inst();
    for (size_t i = 0; i < MENU_LIST_ONSCREEN_ITEMS; i++) {
        menu_list_item_texts[i] = get_g_text_box_inst();
    }
    
    
    // layout
    
    // title
    setup_g_text_box(menu_list_title_text, 
        MENU_LIST_TITLE_PADDING, MENU_LIST_TITLE_PADDING, 
        display_area_width() - 1 - MENU_LIST_TITLE_PADDING,
        1, graphics_calculate_foreground_color(menu_list_definition->title_color));
    menu_list_title_text->truncation_mode = TEXT_MARQUEE;
    menu_list_title_text->alignment_mode = TEXT_ALIGN_CENTER;
    g_text_box_print(menu_list_title_text, menu_list_definition->title);

    // title background
    setup_g_rectangle(menu_list_title_bg, 
        0, 0, 
        display_area_width() - 1, 
        MENU_LIST_START_Y - 1, 
        menu_list_definition->title_color, true);
    
    // selection highlight
    setup_g_rectangle(menu_list_selected_item_highlight, 
        0, 0, 
        display_area_width() - 1 - MENU_LIST_SCROLL_BAR_WIDTH, 0, 
        MENU_LIST_SELECTION_HIGHLIGHT_COLOR, true);

    // scroll bar
    setup_g_rectangle(menu_list_scroll_bar_fill, 
        display_area_width() - MENU_LIST_SCROLL_BAR_WIDTH, 0,
        display_area_width() - 1, 0, 
        MENU_LIST_SCROLL_BAR_FILL_COLOR, true);
    
    // on-screen items
    for (size_t i = 0; i < MENU_LIST_ONSCREEN_ITEMS; i++) {
        text_box = menu_list_item_texts[i];
        setup_g_text_box(text_box, 
            MENU_LIST_ITEM_PADDING, 
            i * MENU_LIST_ITEM_HEIGHT + MENU_LIST_ITEM_PADDING + MENU_LIST_START_Y, 
            display_area_width() - 1 - MENU_LIST_ITEM_PADDING - MENU_LIST_SCROLL_BAR_WIDTH,
            1, 0);
        text_box->truncation_mode = TEXT_MARQUEE;
    }


    graphics_add_rectangle(menu_list_selected_item_highlight);
    for (size_t i = 0; i < MENU_LIST_ONSCREEN_ITEMS; i++) {
        graphics_add_text_box(menu_list_item_texts[i]);
    }
    graphics_add_rectangle(menu_list_scroll_bar_fill);
    graphics_add_rectangle(menu_list_title_bg);
    graphics_add_text_box(menu_list_title_text);
}


menu_list_item_t create_menu_list_item_callback(char* text, void (*callback)()) {
    return (menu_list_item_t){
        text: text,
        action_type: MENU_LIST_CALLBACK,
        action: (menu_list_action_t) callback,
    };
}

menu_list_item_t create_menu_list_item_tree(char* text, menu_list_def_t* menu_def) {
    return (menu_list_item_t){
        text: text,
        action_type: MENU_LIST_TREE,
        action: (menu_list_action_t) menu_def,
    };
}

menu_list_item_t create_menu_list_item_tree_back(char* text) {
    return (menu_list_item_t){
        text: text,
        action_type: MENU_LIST_TREE_BACK,
    };
}

menu_list_def_t* create_menu_list(char* title, color_t title_color, size_t items_size, menu_list_item_t items[]) {
    menu_list_item_t* items_h = malloc(items_size * sizeof(menu_list_item_t));

    for (size_t i = 0; i < items_size; i++) {
        items_h[i] = items[i];
    }

    menu_list_def_t* def_ptr = malloc(sizeof(menu_list_def_t));
    
    *def_ptr = (menu_list_def_t){
        title: title,
        title_color: title_color,
        items: items_h,
        items_size: items_size,
    };
    
    return def_ptr;
}

void menu_list_set(menu_list_def_t* def) {
    menu_list_definition = def;
    menu_list_tree_back_stack_index = 0;
}


menu_binding_t defused_menu_list_binding = {
    display_update_interval: 1000000,
    burn_margin_x: MENU_LIST_HORIZONTAL_BURN_MARGIN,
    burn_margin_y: MENU_LIST_VERTICAL_BURN_MARGIN,

    on_button_event: &defused_menu_list_on_button_event,

    on_nav_up: &defused_menu_list_on_nav_up,
    on_nav_down: &defused_menu_list_on_nav_down,
    on_pre_select: &defused_menu_list_on_pre_select,
    on_cancel_select: &defused_menu_list_on_cancel_select,
    on_select: &defused_menu_list_on_select,
    on_select_held: &defused_menu_list_on_select_held,
    
    update_display: &defused_menu_list_update_display,
    
    init: &defused_menu_list_init
};

void bind_menu_list() {
    return defused_bind(&defused_menu_list_binding);
}
