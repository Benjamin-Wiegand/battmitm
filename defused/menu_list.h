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
#include "defused/gui.h"
#include <stddef.h>


struct menu_list_def;
typedef struct menu_list_def menu_list_def_t;

enum menu_list_action_type {
    MENU_LIST_CALLBACK,
    MENU_LIST_TREE,
    MENU_LIST_TREE_BACK,
};

typedef enum menu_list_action_type menu_list_action_type_t;

union menu_list_action {
    void (*callback)();
    menu_list_def_t* tree;
};

typedef union menu_list_action menu_list_action_t;

struct menu_list_item {
    char* text;

    menu_list_action_type_t action_type;
    menu_list_action_t action;
};

typedef struct menu_list_item menu_list_item_t;

struct menu_list_def {
    char* title;
    uint16_t title_color;

    menu_list_item_t* items;
    size_t items_size;
};

menu_list_item_t create_menu_list_item_callback(char* text, void (*callback)());
menu_list_item_t create_menu_list_item_tree(char* text, menu_list_def_t* menu_def);
menu_list_item_t create_menu_list_item_tree_back(char* text);

menu_list_def_t* create_menu_list(char* title, uint16_t title_color, size_t items_size, menu_list_item_t items[]);

void menu_list_set(menu_list_def_t* def);

void bind_menu_list();
