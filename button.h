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
#define BUTTON_DEBOUNCE_THRESHOLD 1000  // min delay in microseconds from button up/down transition to event firing
#define BUTTON_REPEAT_INTERVAL 50000    // in microseconds
#define BUTTON_REPEAT_DELAY 1000000     // in microseconds


enum button_event {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_DOWN_REPEAT
};

typedef enum button_event button_event_t;

enum button_func {
    BUTTON_NAV_UP,
    BUTTON_SELECT,
    BUTTON_NAV_DOWN,
    
    BUTTON_FUNCTIONS
};

typedef enum button_func button_func_t;

typedef void (*button_callback_t)(button_func_t function, button_event_t event);


button_event_t button_get_state(button_func_t function);

void button_set_callback(button_callback_t callback);
void init_button();

