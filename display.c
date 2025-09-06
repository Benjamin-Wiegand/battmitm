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
#include "display.h"
#include "font.h"
#include "config.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "pico/rand.h"
#include <string.h>
#include <stdio.h>


#define DISPLAY_CMD_ROW_ADDRESS 0x75
#define DISPLAY_CMD_COLUMN_ADDRESS 0x15
#define DISPLAY_CMD_REMAP_AND_DATA_FORMAT 0xA0
#define DISPLAY_CMD_CONTRAST_A 0x81
#define DISPLAY_CMD_CONTRAST_B 0x82
#define DISPLAY_CMD_CONTRAST_C 0x83
#define DISPLAY_CMD_DISPLAY_ON 0xAF
#define DISPLAY_CMD_DISPLAY_OFF 0xAE

#define DISPLAY_CMD_DRAW_LINE 0x21
#define DISPLAY_CMD_DRAW_RECTANGLE 0x22
#define DISPLAY_CMD_COPY 0x23
#define DISPLAY_CMD_FILL 0x26
#define DISPLAY_CMD_CLEAR_WINDOW 0x25


uint16_t display_framebuffer[DISPLAY_RESOLUTION_WIDTH][DISPLAY_RESOLUTION_HEIGHT];

bool display_cs_state = 0;
bool display_dc_state = 0;

bool display_rect_fill_mode = false;

// burn-in protection
uint8_t burn_limit_x = 0;
uint8_t burn_limit_y = 0;
uint8_t burn_offset_x = 0;
uint8_t burn_offset_y = 0;
uint64_t burn_last_updated = 0;

// text rendering
uint8_t text_start_x = 0;
uint8_t text_pos_x = 0;
uint8_t text_pos_y = FONT_HEIGHT;
uint8_t text_limit_x = DISPLAY_RESOLUTION_WIDTH;
uint8_t text_line_spacing = 3;
bool text_wrap = false;
uint8_t text_scale = 1;
uint16_t text_color = COLOR_WHITE;


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
// here is some (roughly) equivalent python code for converting colors:
/*
def rgb888_to_565(color):
    ret = 0
    ret += int(((color >> 16) & 0xFF) * 0x1F / 0xFF) << 11    # red
    ret += int(((color >> 8) & 0xFF) * 0x3F / 0xFF) << 5      # green
    ret += int((color & 0xFF) * 0x1F / 0xFF)                  # blue
    return ret

print(hex(rgb888_to_565(0x696969)))
*/
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
    if (display_rect_fill_mode == enabled) return;
    display_rect_fill_mode = enabled;
    display_send_cmd(DISPLAY_CMD_FILL);
    display_send_cmd(enabled);
}

void display_draw_rectangle_accellerated(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t stroke_color, uint16_t fill_color) {
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
    sleep_ms(1);
}

void display_copy_accellerated(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dest_x, uint8_t dest_y) {
    display_send_cmd(DISPLAY_CMD_COPY);

	display_send_cmd(x1);
	display_send_cmd(y1);
	display_send_cmd(x2);
	display_send_cmd(y2);
    
    display_send_cmd(dest_x);
    display_send_cmd(dest_y);
    sleep_ms(1);
}

void display_shift_accellerated(int x, int y, uint16_t negative_color) {
    if (x == 0 && y == 0) return;
    uint x_offset, y_offset;
    uint8_t x_start, y_start;
    if (x > 95 || x < -95) x %= 96;
    if (y > 63 || y < -63) x %= 64;

    x_start = x < 0 ? -x : 0;
    y_start = y < 0 ? -y : 0;
    x_offset = x < 0 ? 0 : x;
    y_offset = y < 0 ? 0 : y;

    display_copy_accellerated(x_start, y_start, 95, 63, x_offset, y_offset);

    if (x < 0) display_draw_rectangle_accellerated(96 - x_start, 0, 95, 63, negative_color, negative_color);
    else if (x > 0) display_draw_rectangle_accellerated(0, 0, x_offset - 1, 63, negative_color, negative_color);
    if (y < 0) display_draw_rectangle_accellerated(0, 63 - y_start, 95, 63, negative_color, negative_color);
    else if (y > 0) display_draw_rectangle_accellerated(0, 0, 95, y_offset - 1, negative_color, negative_color);
}


void display_draw_rectangle_outline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color) {
    display_draw_line(x1, y1, x2, y1, color);   // top
    display_draw_line(x2, y1, x2, y2, color);   // right
    display_draw_line(x1, y2, x2, y2, color);   // bottom
    display_draw_line(x1, y1, x1, y2, color);   // left
}

void display_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color) {
    if (x2 < x1) {
        uint8_t xt = x2;
        x2 = x1;
        x1 = xt;
    }
    if (y2 < y1) {
        uint8_t yt = y2;
        y2 = y1;
        y1 = yt;
    }
    if (x1 >= DISPLAY_RESOLUTION_WIDTH || y1 >= DISPLAY_RESOLUTION_HEIGHT) return;
    if (x2 >= DISPLAY_RESOLUTION_WIDTH) x2 = DISPLAY_RESOLUTION_WIDTH - 1;
    if (y2 >= DISPLAY_RESOLUTION_HEIGHT) y2 = DISPLAY_RESOLUTION_HEIGHT - 1;

    for (uint x = x1; x <= x2; x++) {
        for (uint y = y1; y <= y2; y++) {
            display_framebuffer[x][y] = color;
        }
    }
}

void display_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t color) {
    if (x2 < x1) {
        uint8_t xt = x2;
        x2 = x1;
        x1 = xt;
    }
    if (y2 < y1) {
        uint8_t yt = y2;
        y2 = y1;
        y1 = yt;
    }
    if (x1 >= DISPLAY_RESOLUTION_WIDTH || y1 >= DISPLAY_RESOLUTION_HEIGHT) return;
    
    if (x1 == x2) {
        // vertical line
        if (y2 >= DISPLAY_RESOLUTION_HEIGHT) y2 = DISPLAY_RESOLUTION_HEIGHT - 1;
        for (uint y = y1; y <= y2; y++) {
            display_framebuffer[x1][y] = color;
        }
    } else if (y1 == y2) {
        // horizontal line
        if (x2 >= DISPLAY_RESOLUTION_WIDTH) x2 = DISPLAY_RESOLUTION_WIDTH - 1;
        for (uint x = x1; x <= x2; x++) {
            display_framebuffer[x][y1] = color;
        }
    } else if (x2 - x1 > y2 - y1) {
        // horizontal-ish line
        uint y;
        for (uint x = x1; x <= x2; x++) {
            y = y1 + (y2 - y1 + 1) * (x - x1) / (x2 - x1 + 1);
            display_draw_pixel(x, y, color);    // checks bounds
        }
    } else {
        // vertical-ish line
        uint x;
        for (uint y = y1; y <= y2; y++) {
            x = x1 + (x2 - x1 + 1) * (y - y1) / (y2 - y1 + 1);
            display_draw_pixel(x, y, color);    // checks bounds
        }
    }
}


void display_clear() {
    display_draw_rectangle(0, 0, DISPLAY_RESOLUTION_WIDTH - 1, DISPLAY_RESOLUTION_HEIGHT - 1, 0);
}

// sets display contrast for all 3 colors
void display_set_contrast(uint8_t contrast) {
    display_send_cmd(DISPLAY_CMD_CONTRAST_A);
    display_send_cmd(contrast);

    display_send_cmd(DISPLAY_CMD_CONTRAST_B);
    display_send_cmd(contrast);

    display_send_cmd(DISPLAY_CMD_CONTRAST_C);
    display_send_cmd(contrast);
}

// update burn-in reduction offset (if it's time)
// if shift_content is true, the display framebuffer will automatically be moved to the new offset
void display_burn_update(bool shift_content) {
    uint64_t timestamp = time_us_64();
    if (burn_last_updated + DISPLAY_BURN_SHIFT_MIN_INTERVAL > timestamp) return;
    burn_last_updated = timestamp;

    int shift_x = (get_rand_32() % 3) - 1;
    if (burn_limit_x == 0) shift_x = 0;
    if (burn_offset_x == 0 && shift_x < 0) shift_x *= -1;
    if (burn_offset_x >= burn_limit_x && shift_x > 0) shift_x *= -1;

    int shift_y = (get_rand_32() % 3) - 1;
    if (burn_limit_y == 0) shift_y = 0;
    if (burn_offset_y == 0 && shift_y < 0) shift_y *= -1;
    if (burn_offset_y >= burn_limit_y && shift_y > 0) shift_y *= -1;
    
    burn_offset_x += shift_x;
    burn_offset_y += shift_y;
    
    if (shift_content) display_shift_accellerated(shift_x, shift_y, 0);
}

// set maximum offset for burn-in reduction
void display_set_burn_limits(uint8_t x_limit, uint8_t y_limit) {
    burn_limit_x = x_limit;
    burn_limit_y = y_limit;
    if (burn_offset_x > x_limit) burn_offset_x = x_limit;
    if (burn_offset_y > y_limit) burn_offset_y = y_limit;
}

// width of usable display area (accounting for burn limits)
uint8_t display_area_width() {
    return DISPLAY_RESOLUTION_WIDTH - burn_limit_x;
}

// height of usable display area (accounting for burn limits)
uint8_t display_area_height() {
    return DISPLAY_RESOLUTION_HEIGHT - burn_limit_y;
}


void display_draw_pixel(uint8_t x, uint8_t y, uint16_t color) {
    if (x >= DISPLAY_RESOLUTION_WIDTH || y >= DISPLAY_RESOLUTION_HEIGHT) return;
    display_framebuffer[x][y] = color;
}


void display_draw_char(uint8_t x_pos, uint8_t y_pos, uint8_t scale_factor, uint16_t color, char c) {
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
                display_draw_pixel(
                    x_pos + x * scale_factor + j % scale_factor, 
                    y_pos + y * scale_factor + j / scale_factor, 
                    color);
            }
        }
    }
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

void display_set_text_bound(uint8_t x_limit) {
    text_limit_x = x_limit;
}

void display_set_text_wrap(bool enabled) {
    text_wrap = enabled;
}

void display_set_text_line_spacing(uint8_t spacing) {
    text_line_spacing = spacing;
}

void display_line_feed() {
    text_pos_y += FONT_HEIGHT * text_scale + text_line_spacing;
    text_pos_x = text_start_x;
}

void display_print(char* text) {
    size_t length = strlen(text);
    for (size_t i = 0; i < length; i++) {
        if (text[i] == '\n') {
            display_line_feed();
            continue;
        }
        
        if (text_wrap && text_pos_x + FONT_WIDTH * text_scale >= text_limit_x) {
            display_line_feed();
        }
        
        display_draw_char(text_pos_x, text_pos_y, text_scale, text_color, text[i]);
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


void set_display_address_window(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    display_send_cmd(DISPLAY_CMD_COLUMN_ADDRESS);
    display_send_cmd(x);
    display_send_cmd(width);

    display_send_cmd(DISPLAY_CMD_ROW_ADDRESS);
    display_send_cmd(y);
    display_send_cmd(height);
}

void display_refresh_region(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    if (x2 < x1) {
        uint8_t xt = x2;
        x2 = x1;
        x1 = xt;
    }
    if (y2 < y1) {
        uint8_t yt = y2;
        y2 = y1;
        y1 = yt;
    }
    if (x1 >= DISPLAY_RESOLUTION_WIDTH || y1 >= DISPLAY_RESOLUTION_HEIGHT) return;
    if (x2 >= DISPLAY_RESOLUTION_WIDTH) x2 = DISPLAY_RESOLUTION_WIDTH - 1;
    if (y2 >= DISPLAY_RESOLUTION_HEIGHT) y2 = DISPLAY_RESOLUTION_HEIGHT - 1;

    uint8_t color_bytes[2];
    for (uint x = x1; x <= x2; x++) {
        for (uint y = y1; y <= y2; y++) {
            set_display_address_window(burn_offset_x + x, burn_offset_y + y, 1, 1);

            // out of bounds, but still clear it
            if (x >= display_area_width() || y >= display_area_height()) {
                color_bytes[0] = 0;
                color_bytes[1] = 0;
                display_send_buffer(color_bytes, 2);
                continue;
            }

            color_bytes[0] = display_framebuffer[x][y] >> 8;
            color_bytes[1] = display_framebuffer[x][y] & 0xFF;
            display_send_buffer(color_bytes, 2);
        }
    }
}

void display_refresh() {
    display_refresh_region(0, 0, DISPLAY_RESOLUTION_WIDTH - 1, DISPLAY_RESOLUTION_HEIGHT - 1);
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
    display_clear();
    display_refresh();
}

