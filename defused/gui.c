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
#include "defused/gui.h"
#include "display.h"

struct repeating_timer display_update_timer;

void button_callback(button_func_t function, button_event_t event) {
    switch (event) {
        case BUTTON_UP:
        case BUTTON_DOWN:
        case BUTTON_DOWN_REPEAT:
        default:
            break;
    }
}

bool display_update_handler(struct repeating_timer* t) {

}

void defused_set_display_update_interval(uint64_t interval) {
    add_repeating_timer_us(interval, display_update_handler, NULL, &display_update_timer);
}

void init_gui() {
    init_button();
    init_display();

    button_set_callback(&button_callback);

    display_set_text_position(1, 40);
    display_set_text_color(COLOR_WHITE);
    display_set_text_scale(2);
    display_print("BattMITM");
    
    sleep_ms(2000);
    
    display_clear();

}
