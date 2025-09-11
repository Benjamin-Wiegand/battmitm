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
#include "defused/main_menu.h"
#include "defused/gui.h"
#include "display.h"

#include "defused/menu_list.h"
#include "defused/stat_browser.h"

menu_list_def_t* main_menu_def = NULL;

void init_main_menu() {
    if (main_menu_def != NULL) return;  // only do this once

    main_menu_def = create_menu_list("main menu", COLOR_FAINT_BLUE, 2, (menu_list_item_t[]) {

        create_menu_list_item_callback("battery stats", &bind_stat_browser),
        create_menu_list_item_callback("sleep", (void*) &defused_enter_inactive_mode),

    });
}

void bind_main_menu() {
    menu_list_set(main_menu_def);
    bind_menu_list();
}
