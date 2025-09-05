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
#include "aod.h"


menu_binding_t* defused_current_binding = NULL;

uint64_t defused_last_display_update = 0;
bool defused_force_display_update = false;

void button_callback(button_func_t function, button_event_t event) {
    bool handled = defused_current_binding->on_button_event(function, event);
    if (handled) return;
    
    switch (event) {
        case BUTTON_UP:
        case BUTTON_DOWN:
        case BUTTON_DOWN_REPEAT:
        default:
            break;
    }
}

void defused_update_display_now() {
    defused_force_display_update = true;
}

void defused_bind(menu_binding_t* binding) {
    defused_current_binding = binding;
    display_set_burn_limits(binding->burn_margin_x, binding->burn_margin_y);
    defused_current_binding->init();
}


void defused_loop() {
    if (defused_current_binding == NULL) return;
    uint64_t timestamp = time_us_64();

    if (defused_force_display_update || defused_last_display_update + defused_current_binding->display_update_interval < timestamp) {
        defused_force_display_update = false;
        defused_current_binding->update_display();
        defused_last_display_update = timestamp;
    }
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
    
    defused_bind(bind_aod());
    
    while (true) defused_loop();
}
