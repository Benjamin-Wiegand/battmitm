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
#include "button.h"
#include "config.h"
#include "pico/stdlib.h"


// stores general button state
// called "button_bouncer" because I think it's funny
struct button_bouncer {
    button_func_t function;
    uint64_t up_timestamp;
    uint64_t down_timestamp;
    button_event_t state;
};

typedef struct button_bouncer button_bouncer_t;


button_callback_t button_callback_internal = NULL;

button_bouncer_t button_bouncers[BUTTON_FUNCTIONS];


button_func_t button_pin_to_func(uint pin) {
    switch (pin) {
        case BUTTON_NAV_UP_PIN: return BUTTON_NAV_UP;
        case BUTTON_SELECT_PIN: return BUTTON_SELECT;
        case BUTTON_NAV_DOWN_PIN: return BUTTON_NAV_DOWN;
        default: return -1;
    }
}


int64_t button_repeat_handler(alarm_id_t id, void* bouncer_ptr) {
    button_bouncer_t* bouncer = bouncer_ptr;
    uint64_t timestamp = time_us_64();

    if (bouncer->state != BUTTON_DOWN) return 0;
    if (bouncer->down_timestamp + BUTTON_REPEAT_DELAY > timestamp) return 0;

    button_callback_internal(bouncer->function, BUTTON_DOWN_REPEAT);
    return -BUTTON_REPEAT_INTERVAL;
}


int64_t button_state_handler(alarm_id_t id, void* bouncer_ptr) {
    button_bouncer_t* bouncer = bouncer_ptr;
    button_event_t event;
    uint64_t timestamp = time_us_64();
    uint64_t leading_timestamp;
    
    if (bouncer->up_timestamp > bouncer->down_timestamp) {          // up
        leading_timestamp = bouncer->up_timestamp;
        event = BUTTON_UP;
    } else if (bouncer->down_timestamp > bouncer->up_timestamp) {   // down
        leading_timestamp = bouncer->down_timestamp;
        event = BUTTON_DOWN;
    } else {
        return 0;
    }

    if (bouncer->state == event) return 0;
    if (leading_timestamp + BUTTON_DEBOUNCE_THRESHOLD > timestamp) return 0;
    bouncer->state = event;
    
    // start repeat
    if (event == BUTTON_DOWN)
        add_alarm_at(timestamp + BUTTON_REPEAT_DELAY, &button_repeat_handler, bouncer, true);

    button_callback_internal(bouncer->function, event);
    return 0;
}


void button_irq_handler(uint pin, uint32_t event_mask) {
    button_func_t function = button_pin_to_func(pin);
    button_bouncer_t* bouncer = &button_bouncers[function];
    uint64_t timestamp = time_us_64();

    if (event_mask & GPIO_IRQ_EDGE_RISE) {          // up
        bouncer->up_timestamp = timestamp;
    } else if (event_mask & GPIO_IRQ_EDGE_FALL) {   // down
        bouncer->down_timestamp = timestamp;
    } else {
        return;
    }
    
    add_alarm_at(timestamp + BUTTON_DEBOUNCE_THRESHOLD, &button_state_handler, bouncer, true);
}

button_event_t button_get_state(button_func_t function) {
    if (function >= BUTTON_FUNCTIONS) return BUTTON_UP;
    if (function < 0) return BUTTON_UP;
    return button_bouncers[function].state;
}

void button_set_callback(button_callback_t callback) {
    button_callback_internal = callback;
}

void button_setup_gpio(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    gpio_set_irq_enabled_with_callback(pin, 0b1100, true, button_irq_handler);
}


void init_button() {
    for (int i = 0; i < BUTTON_FUNCTIONS; i++)
        button_bouncers[i] = (button_bouncer_t){i, 0, 0, BUTTON_UP};

    button_setup_gpio(BUTTON_NAV_UP_PIN);
    button_setup_gpio(BUTTON_SELECT_PIN);
    button_setup_gpio(BUTTON_NAV_DOWN_PIN);
}
