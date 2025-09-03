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

#include "font.h"
#include "config.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>


#define DISPLAY_CMD_ROW_ADDRESS 0x75
#define DISPLAY_CMD_COLUMN_ADDRESS 0x15
#define DISPLAY_CMD_REMAP_AND_DATA_FORMAT 0xA0
#define DISPLAY_CMD_DISPLAY_ON 0xAF
#define DISPLAY_CMD_DISPLAY_OFF 0xAE

#define DISPLAY_CMD_DRAW_LINE 0x21
#define DISPLAY_CMD_DRAW_RECTANGLE 0x22
#define DISPLAY_CMD_COPY 0x23
#define DISPLAY_CMD_FILL 0x26


bool display_cs_state = 0;
bool display_dc_state = 0;

uint8_t text_start_x = 0;
uint8_t text_pos_x = 0;
uint8_t text_pos_y = FONT_HEIGHT;
uint8_t text_scale = 1;
uint16_t text_color = 0xFFFF;


void display_set_cs(bool state) {
#if DISPLAY_CS_PIN != -1
    if (display_cs_state == state) return;
    gpio_put(DISPLAY_CS_PIN, state);
    display_cs_state = state;
#endif
}

void display_set_dc(bool state) {
    if (display_dc_state == state) return;
    gpio_put(DISPLAY_DC_PIN, state);
    display_dc_state = state;
}


void display_send_cmd(uint8_t cmd) {
    display_set_dc(0);
    display_set_cs(0);
    spi_write_blocking(DISPLAY_SPI, &cmd, 1);
}

void display_send_buffer(uint8_t* buffer, size_t length) {
    display_set_dc(1);
    display_set_cs(0);
    spi_write_blocking(DISPLAY_SPI, buffer, length);
}


// converts rgb888 (24-bit) colors to rgb565 (16-bit) colors
// provided for convenience during development since some know rgb888 by heart
// avoid using this when you can, and just use rgb565 colors directly
uint16_t rgb888_to_565(uint32_t color24) {
    uint16_t color16 = 0;
    color16 += (((color24 >> 16) & 0xFF) * 0x1F / 0xFF) << 11;  // red
    color16 += (((color24 >> 8) & 0xFF) * 0x3F / 0xFF) << 5;    // green
    color16 += (color24 & 0xFF) * 0x1F / 0xFF;                  // blue
    return color16;
}

uint8_t rgb565_red(uint16_t color16) {
    return (color16 >> 10) & 0x3E;
}

uint8_t rgb565_green(uint16_t color16) {
    return (color16 >> 5) & 0x3F;
}

uint8_t rgb565_blue(uint16_t color16) {
    return (color16 & 0x1F) << 1;
}

void display_set_rectangle_fill(bool enabled) {
    display_send_cmd(DISPLAY_CMD_FILL);
    display_send_cmd(enabled);
}

void display_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t stroke_color, uint16_t fill_color) {
	display_send_cmd(DISPLAY_CMD_DRAW_RECTANGLE);

	display_send_cmd(x1);
	display_send_cmd(y1);
	display_send_cmd(x2);
	display_send_cmd(y2);

    display_send_cmd(rgb565_red(stroke_color));
    display_send_cmd(rgb565_green(stroke_color));
    display_send_cmd(rgb565_blue(stroke_color));

    display_send_cmd(rgb565_red(fill_color));
    display_send_cmd(rgb565_green(fill_color));
    display_send_cmd(rgb565_blue(fill_color));
}

void display_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color) {
	display_send_cmd(DISPLAY_CMD_DRAW_LINE);

	display_send_cmd(x1);
	display_send_cmd(y1);
	display_send_cmd(x2);
	display_send_cmd(y2);

    display_send_cmd(rgb565_red(color));
    display_send_cmd(rgb565_green(color));
    display_send_cmd(rgb565_blue(color));
}


void display_copy(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dest_x, uint8_t dest_y) {
    display_send_cmd(DISPLAY_CMD_COPY);

	display_send_cmd(x1);
	display_send_cmd(y1);
	display_send_cmd(x2);
	display_send_cmd(y2);
    
    display_send_cmd(dest_x);
    display_send_cmd(dest_y);
}

void display_shift(int x, int y, uint16_t negative_color) {
    uint x_offset, y_offset;
    uint8_t x_start, y_start;
    if (x > 95 || x < -95) x %= 96;
    if (y > 63 || y < -63) x %= 64;

    x_start = x < 0 ? -x : 0;
    y_start = y < 0 ? -y : 0;
    x_offset = x < 0 ? 0 : x;
    y_offset = y < 0 ? 0 : y;

    display_copy(x_start, y_start, 95, 63, x_offset, y_offset);

    display_set_rectangle_fill(true);
    if (x < 0) display_draw_rectangle(96 - x_start, 0, 95, 63, negative_color, negative_color);
    else if (x > 0) display_draw_rectangle(0, 0, x_offset - 1, 63, negative_color, negative_color);
    if (y < 0) display_draw_rectangle(0, 63 - y_start, 95, 63, negative_color, negative_color);
    else if (y > 0) display_draw_rectangle(0, 0, 95, y_offset - 1, negative_color, negative_color);

}


void set_display_address_window(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    display_send_cmd(DISPLAY_CMD_COLUMN_ADDRESS);
    display_send_cmd(x);
    display_send_cmd(width);

    display_send_cmd(DISPLAY_CMD_ROW_ADDRESS);
    display_send_cmd(y);
    display_send_cmd(height);
}


void draw_char(uint8_t x_pos, uint8_t y_pos, uint8_t scale_factor, uint16_t color, char c) {
    uint8_t color_bytes[2] = {color >> 8, color & 0xFF};
    uint8_t* char_data = font_get_char(c);
    uint i;

    // position relative to "text line"
    y_pos -= scale_factor * FONT_HEIGHT;

    // for whatever reason if I push it all as one big buffer it doesn't work
    for (uint x = 0; x < FONT_WIDTH; x++) {
        for (uint y = 0; y < FONT_HEIGHT; y++) {
            i = x + y * FONT_WIDTH;
            if (!((char_data[i / 8] >> (7 - i % 8)) & 1)) continue;

            for (uint j = 0; j < scale_factor * scale_factor; j++) {
                set_display_address_window(
                    x_pos + x * scale_factor + j % scale_factor, 
                    y_pos + y * scale_factor + j / scale_factor, 
                    1, 1);
                display_send_buffer(color_bytes, 2);
            }
        }
    }
}

void display_draw_pixel(uint8_t x, uint8_t y, uint16_t color) {
    uint8_t color_bytes[2] = {color >> 8, color & 0xFF};
    set_display_address_window(x, y, 1, 1); 
    display_send_buffer(color_bytes, 2);
}


void display_set_text_color(uint16_t color) {
    text_color = color;
}

void display_set_text_scale(uint8_t scale_factor) {
    text_scale = scale_factor;
}

void display_set_text_position(uint8_t x, uint8_t y) {
    text_start_x = x;
    text_pos_x = x;
    text_pos_y = y;
}

void display_print(char* text) {
    size_t length = strlen(text);
    for (size_t i = 0; i < length; i++) {
        if (text[i] == '\n') {
            text_pos_y += FONT_HEIGHT * text_scale + text_scale;
            text_pos_x = text_start_x;
            continue;
        }
        draw_char(text_pos_x, text_pos_y, text_scale, text_color, text[i]);
        text_pos_x += FONT_WIDTH * text_scale + text_scale;
    }
}

void display_printf(char* text, ...) {
    va_list args;
    va_start(args, text);

    // this is terrible, but I don't like wasting ram.
    size_t len = vsprintf(NULL, text, args);
    char formatted[len];
    vsprintf(formatted, text, args);
    va_end(args);

    display_print(formatted);
}


void init_display() {
    // data control
    gpio_init(DISPLAY_DC_PIN);
    gpio_set_dir(DISPLAY_DC_PIN, GPIO_OUT);
    display_set_dc(0);
    
    // chip select (optional)
#if DISPLAY_CS_PIN != -1
    gpio_init(DISPLAY_CS_PIN);
    gpio_set_dir(DISPLAY_CS_PIN, GPIO_OUT);
    display_set_cs(0);
#endif

    // spi
    gpio_set_function(DISPLAY_SDA_PIN, GPIO_FUNC_SPI);
    gpio_set_function(DISPLAY_SCL_PIN, GPIO_FUNC_SPI);
    spi_init(DISPLAY_SPI, DISPLAY_SPI_BAUD);
    spi_set_format(DISPLAY_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // reset
    gpio_init(DISPLAY_RESET_PIN);
    gpio_set_dir(DISPLAY_RESET_PIN, GPIO_OUT);
    gpio_put(DISPLAY_RESET_PIN, 0);
    sleep_ms(100);
    gpio_put(DISPLAY_RESET_PIN, 1);

    // setup
    display_send_cmd(DISPLAY_CMD_DISPLAY_OFF);

    display_send_cmd(DISPLAY_CMD_REMAP_AND_DATA_FORMAT);
    uint8_t format = 0b01100000;
#if !DISPLAY_FLIP_180
    format |= 0b00010010;
#endif
    display_send_cmd(format);

    display_send_cmd(DISPLAY_CMD_DISPLAY_ON);

    // clear framebuffer
    display_set_rectangle_fill(true);
    display_draw_rectangle(0, 0, 95, 63, 0, 0);
}

