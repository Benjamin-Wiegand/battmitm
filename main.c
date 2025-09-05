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
#include "mitm.h"
#include "status.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "display.h"
#include "button.h"

#include "battery.h"
#include "smbus.h"

bool do_update = false;
uint8_t button_down_ctrs[] = {0, 0, 0};
uint8_t button_up_ctrs[] = {0, 0, 0};

bool trigger_display_update(struct repeating_timer* t) {
    do_update = true;
    return true;
}

void update_display() {
    display_burn_update(false);

    display_clear();

    display_set_contrast(0); // dimmest setting
    display_set_text_position(0, 14);
    display_set_text_color(0xFFFF);
    display_set_text_scale(2);
    
    battery_stat_t* stat;
    stat = battery_get_stat(BATT_CMD_RELATIVE_STATE_OF_CHARGE);
    if (battery_stat_is_expired(stat))
        display_print("loading...");
    else if (battery_stat_is_error(stat))
        display_print("error");
    else 
        display_printf("%d%%", *((uint16_t*) stat->cached_result));

    display_set_text_scale(1);
    display_print("\n");

    stat = battery_get_stat(BATT_CMD_VOLTAGE);
    if (battery_stat_is_expired(stat))
        display_print("loading...");
    else if (battery_stat_is_error(stat))
        display_print("error");
    else 
        display_printf("%.3f V", (*((uint16_t*) stat->cached_result)) / 1000.0);

    display_print("\n");

    stat = battery_get_stat(BATT_CMD_CURRENT);
    if (battery_stat_is_expired(stat))
        display_print("loading...");
    else if (battery_stat_is_error(stat))
        display_print("error");
    else 
        display_printf("%.3f A", (*((int16_t*) stat->cached_result)) / 1000.0);

    display_print("\n");

    display_print(button_get_state(BUTTON_NAV_UP) == BUTTON_DOWN ? " D " : " U ");
    display_print(button_get_state(BUTTON_SELECT) == BUTTON_DOWN ? "  D " : "  U ");
    display_print(button_get_state(BUTTON_NAV_DOWN) == BUTTON_DOWN ? "  D " : "  U ");
    display_print("\n");
    
    display_printf("%03d %03d %03d\n", button_down_ctrs[0], button_down_ctrs[1], button_down_ctrs[2]);
    display_printf("%03d %03d %03d\n", button_up_ctrs[0], button_up_ctrs[1], button_up_ctrs[2]);

}

void button_callback(button_func_t function, button_event_t event) {
    switch (event) {
        case BUTTON_UP:
            button_up_ctrs[function]++;
            break;
        case BUTTON_DOWN:
        case BUTTON_DOWN_REPEAT:
            button_down_ctrs[function]++;
            break;
        default:
            break;
    }
    do_update = true;
}

int main() {
    stdio_init_all();
    init_status();
    init_display();
    init_battery();
    init_button();

    button_set_callback(&button_callback);
    
    display_set_text_position(1, 40);
    display_set_text_color(0xFFFF);
    display_set_text_scale(2);
    display_print("BattMITM");

    display_set_burn_limits(5, 5);

    struct repeating_timer display_update_timer;
    add_repeating_timer_ms(2000, trigger_display_update, NULL, &display_update_timer);

    init_mitm();
    while (true) {
        mitm_loop();
        battery_update_cache();
        if (do_update) {
            do_update = false;
            update_display();
        }
    }
}
