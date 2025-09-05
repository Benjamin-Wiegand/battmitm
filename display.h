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
#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_RESOLUTION_WIDTH 96
#define DISPLAY_RESOLUTION_HEIGHT 64

uint16_t rgb888_to_565(uint32_t color24);

void display_set_rectangle_fill(bool enabled);
void display_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t stroke_color, uint16_t fill_color);
void display_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color);
void display_draw_char(uint8_t x_offset, uint8_t y_offset, uint8_t scale_factor, uint16_t color, char c);
void display_draw_pixel(uint8_t x, uint8_t y, uint16_t color);

void display_copy(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dest_x, uint8_t dest_y);

void display_burn_update(bool shift_content);
void display_set_burn_limits(uint8_t x_limit, uint8_t y_limit);
uint8_t display_area_width();
uint8_t display_area_height();
    
void display_set_text_color(uint16_t color);
void display_set_text_scale(uint8_t scale_factor);
void display_set_text_position(uint8_t x, uint8_t y);

void display_print(char* text);
void display_printf(char* text, ...);
    
void init_display();