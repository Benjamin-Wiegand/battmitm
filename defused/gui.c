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
#include "defused/aod.h"
#include "defused/stat_browser.h"
#include "display.h"
#include "config.h"


// active menu
menu_binding_t* defused_current_binding = NULL;

// inactive mode (aod)
uint64_t defused_last_interaction = 0;
bool defused_inactive_mode = false;

// contrast attenuation
uint8_t defused_contrast_current = DISPLAY_CONTRAST;
uint8_t defused_contrast_target = DISPLAY_CONTRAST;
uint64_t defused_contrast_last_attenuated = 0;

// display updates
uint64_t defused_last_display_update = 0;
bool defused_force_display_update = false;

// select button state
bool defused_pre_selecting = false;
bool defused_long_selected = false;
bool defused_long_select_captured = false;

void button_callback(button_func_t function, button_event_t event) {
    defused_last_interaction = time_us_64();
    if (event == BUTTON_DOWN && defused_exit_inactive_mode()) return;
    
    if (defused_current_binding == NULL) return;
    if (defused_current_binding->on_button_event(function, event)) return;

    switch (function) {
        case BUTTON_NAV_UP:
            switch (event) {
                case BUTTON_UP:
                    break;
                case BUTTON_DOWN:
                case BUTTON_DOWN_REPEAT:
                    if (defused_pre_selecting) {
                        defused_pre_selecting = false;
                        defused_current_binding->on_cancel_select();
                    }
                    defused_current_binding->on_nav_up();
                    break;
                default:
                    break;
            }
            break;
        case BUTTON_SELECT:
            switch (event) {
                case BUTTON_UP:
                    if (defused_long_selected && defused_long_select_captured) {
                        defused_long_selected = false;
                        break;
                    }
                    if (!defused_pre_selecting) break;
                    defused_current_binding->on_select();
                    break;
                case BUTTON_DOWN:
                    defused_pre_selecting = true;
                    defused_long_selected = false;
                    defused_current_binding->on_pre_select();
                    break;
                case BUTTON_DOWN_REPEAT:
                    if (defused_long_selected) break;
                    if (!defused_pre_selecting) break;
                    defused_long_selected = true;
                    defused_long_select_captured = defused_current_binding->on_select_held();
                    if (defused_long_select_captured) defused_pre_selecting = false;
                    break;
                default:
                    break;
            }
            break;
        case BUTTON_NAV_DOWN:
            switch (event) {
                case BUTTON_UP:
                    break;
                case BUTTON_DOWN:
                case BUTTON_DOWN_REPEAT:
                    if (defused_pre_selecting) {
                        defused_pre_selecting = false;
                        defused_current_binding->on_cancel_select();
                    }
                    defused_current_binding->on_nav_down();
                    break;
                default:
                    break;
            }
            break;
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

    defused_update_display_now();
}

bool defused_enter_inactive_mode() {
    if (defused_inactive_mode) return false;
    defused_inactive_mode = true;
    defused_attenuate_contrast(DISPLAY_CONTRAST_INACTIVE);
    defused_bind(bind_aod());
    return true;
}

bool defused_exit_inactive_mode() {
    if (!defused_inactive_mode) return false;
    defused_last_interaction = time_us_64();
    defused_inactive_mode = false;
    defused_attenuate_contrast(DISPLAY_CONTRAST);
    defused_bind(bind_stat_browser());
    return true;
}


void defused_attenuate_contrast(uint8_t target) {
    defused_contrast_target = target;
}


void defused_loop() {
    if (defused_current_binding == NULL) return;
    uint64_t timestamp = time_us_64();

    if (!defused_inactive_mode && defused_last_interaction + DISPLAY_INACTIVITY_TIMEOUT < timestamp) {
        defused_enter_inactive_mode();
    }

    if (defused_force_display_update || defused_last_display_update + defused_current_binding->display_update_interval < timestamp) {
        defused_force_display_update = false;
        defused_current_binding->update_display();
        defused_last_display_update = timestamp;
    }
    
    if (defused_contrast_current != defused_contrast_target && defused_contrast_last_attenuated + DISPLAY_CONTRAST_ATTENUATION_INTERVAL < timestamp ) {
        defused_contrast_last_attenuated = timestamp;
        if (defused_contrast_current > defused_contrast_target) defused_contrast_current--;
        else defused_contrast_current++;
        display_set_contrast(defused_contrast_current);
    }
}

void init_gui() {
    init_button();
    init_display();

    button_set_callback(&button_callback);

    display_set_contrast(defused_contrast_current);
    display_set_text_position(1, 40);
    display_set_text_color(COLOR_WHITE);
    display_set_text_scale(2);
    display_print("BattMITM");
    
    sleep_ms(2000);
    
    display_clear();
    
    defused_bind(bind_stat_browser());
    
    while (true) defused_loop();
}
