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
#include <stddef.h>

// to keep everything *mostly* statically allocated, there are a fixed number of instances allocated for each graphics object
// note: strings are still dynamically allocated
#define GRAPHICS_MAX_TEXT_BOXES     18  // assuming 6 lines, this is 3 text boxes per line (excessive)
#define GRAPHICS_MAX_RECTS          8
#define GRAPHICS_MAX_LINES          8
#define GRAPHICS_MAX_OBJECTS        GRAPHICS_MAX_TEXT_BOXES + GRAPHICS_MAX_RECTS + GRAPHICS_MAX_LINES

#define MARQUEE_START_DELAY 2000000
#define MARQUEE_INTERVAL 100000

typedef uint8_t coord_t;
typedef uint16_t color_t;

struct _g_marquee_state {
    bool enabled;
    size_t index;
    uint64_t last_updated;
};
typedef struct _g_marquee_state _g_marquee_state_t;

enum g_text_truncation_mode {
    TEXT_WRAP,
    TEXT_MARQUEE,
    TEXT_TRUNCATE,
};
typedef enum g_text_truncation_mode g_text_truncation_mode_t;

enum g_text_alignment_mode {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT,
};
typedef enum g_text_alignment_mode g_text_alignment_mode_t;

struct g_text_box {
    bool enabled;

    char* text;
    size_t length;
    
    coord_t x1;
    coord_t y1;
    coord_t x2;
    coord_t max_lines;

    coord_t line_spacing;
    coord_t scale_factor;
    color_t color;

    g_text_alignment_mode_t alignment_mode;
    g_text_truncation_mode_t truncation_mode;

    _g_marquee_state_t _marquee;
};
typedef struct g_text_box g_text_box_t;

struct g_rectangle {
    bool enabled;

    coord_t x1;
    coord_t y1;
    coord_t x2;
    coord_t y2;

    color_t color;
    bool filled;
};
typedef struct g_rectangle g_rectangle_t;

struct g_line {
    bool enabled;

    coord_t x1;
    coord_t y1;
    coord_t x2;
    coord_t y2;

    color_t color;
};
typedef struct g_line g_line_t;


coord_t g_text_box_height(g_text_box_t* inst);
void g_text_box_print(g_text_box_t* inst, char* text);
void g_text_box_printf(g_text_box_t* inst, char* text, ...);

g_text_box_t* get_g_text_box_inst();
g_rectangle_t* get_g_rectangle_inst();
g_line_t* get_g_line_inst();

void setup_g_text_box(g_text_box_t* inst, coord_t x1, coord_t y1, coord_t x2, coord_t max_lines, color_t color);
void setup_g_rectangle(g_rectangle_t* inst, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color, bool filled);
void setup_g_line(g_line_t* inst, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color);

// everything is rendered in order. the first to be added go first, and others display over them
void graphics_add_text_box(g_text_box_t* inst);
void graphics_add_rectangle(g_rectangle_t* inst);
void graphics_add_line(g_line_t* inst);

// renders everything to the framebuffer
void graphics_render();

// updates stuff like marquees
void graphics_update();

// resets everything
void graphics_reset();

void init_graphics();
