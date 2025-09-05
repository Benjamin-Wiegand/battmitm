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
// the gui is codename "defused" because I think it's funny

#ifndef MENU_BINDING_DEF
#define MENU_BINDING_DEF

#include "pico/stdlib.h"
#include "button.h"

struct menu_binding {
    uint64_t display_update_interval;
    uint8_t burn_margin_x;
    uint8_t burn_margin_y;

    // returning true will "capture" the event and not propagate to on_navigate or on_select
    bool (*on_button_event)(button_func_t function, button_event_t event);

    void (*on_nav_up)();
    void (*on_nav_down)();
    void (*on_pre_select)();
    void (*on_cancel_select)();
    void (*on_select)();
    
    // return false if no action, and releasing will call on_select
    bool (*on_select_held)();

    void (*update_display)();
    
    void (*init)();
};

typedef struct menu_binding menu_binding_t;

#endif

void defused_update_display_now();

void defused_bind(menu_binding_t* binding);

void init_gui();
