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
#include "graphics.h"
#include "display.h"
#include "font.h"
#include "pico/stdlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


union g_object_ptr {
    g_text_box_t* text_box;
    g_rectangle_t* rectangle;
    g_line_t* line;
};
typedef union g_object_ptr g_object_ptr_t;

enum g_object_type {
    GRAPHICS_TEXT_BOX,
    GRAPHICS_RECTANGLE,
    GRAPHICS_LINE,
};
typedef enum g_object_type g_object_type_t;

struct g_object_holder {
    g_object_type_t type;
    g_object_ptr_t ptr;
};
typedef struct g_object_holder g_object_holder_t;


g_text_box_t graphics_text_box_pool[GRAPHICS_MAX_TEXT_BOXES];
g_rectangle_t graphics_rectangle_pool[GRAPHICS_MAX_RECTS];
g_line_t graphics_line_pool[GRAPHICS_MAX_LINES];
size_t graphics_text_box_alloc_index = 0;
size_t graphics_rectangle_alloc_index = 0;
size_t graphics_line_alloc_index = 0;

g_object_holder_t graphics_objects_internal[GRAPHICS_MAX_OBJECTS];
size_t graphics_object_count = 0;


void init_g_text_box(g_text_box_t* inst) {
    inst->enabled = true;
    if (inst->text != NULL && inst->length > 0) free(inst->text);
    inst->text = NULL;
    inst->length = 0;
    inst->x1 = 0;
    inst->y1 = 0;
    inst->x2 = 0;
    inst->max_lines = 1;
    inst->line_spacing = 3;
    inst->scale_factor = 1;
    inst->color = COLOR_WHITE;
    inst->alignment_mode = TEXT_ALIGN_LEFT;
    inst->truncation_mode = TEXT_WRAP;
    inst->_marquee = (_g_marquee_state_t){
        index: 0,
        last_updated: 0,
    };
}

void init_g_rectangle(g_rectangle_t* inst) {
    inst->enabled = true;
    inst->x1 = 0;
    inst->y1 = 0;
    inst->x2 = 0;
    inst->y2 = 0;
    inst->color = COLOR_WHITE;
    inst->filled = true;
}

void init_g_line(g_line_t* inst) {
    inst->enabled = true;
    inst->x1 = 0;
    inst->y1 = 0;
    inst->x2 = 0;
    inst->y2 = 0;
    inst->color = COLOR_WHITE;
}

coord_t g_text_box_chars_per_line(g_text_box_t* inst) {
    coord_t char_width = FONT_WIDTH * inst->scale_factor;
    coord_t tb_width = inst->x2 - inst->x1 + 1;
    coord_t chars_per_line = 1 + ((tb_width - char_width) / (char_width + inst->scale_factor)); // accounts for spacing
    return chars_per_line < 1 ? 1 : chars_per_line; // at least 1 char will be rendered
}

void graphics_render_text_line_internal(coord_t x, coord_t y, coord_t x_limit, g_text_alignment_mode_t alignment_mode, coord_t scale_factor, color_t color, char* text, size_t length, bool marquee_start, bool marquee_end) {
    coord_t line_length = FONT_WIDTH * scale_factor * length + scale_factor * (length - 1);
    switch (alignment_mode) {
        case TEXT_ALIGN_CENTER:
            x = (x_limit - x + 1 - line_length) / 2;
            break;

        case TEXT_ALIGN_RIGHT:
            x = x_limit - line_length + 1;
            break;

        case TEXT_ALIGN_LEFT:
        default:
            break;
    }
    
    if (marquee_start) {
        display_draw_char(x, y, scale_factor, COLOR_WHITE, '<');
        x += FONT_WIDTH * scale_factor + scale_factor;        
    }

    for (size_t i = marquee_start; i < length - marquee_end; i++) {
        display_draw_char(x, y, scale_factor, color, text[i]);
        x += FONT_WIDTH * scale_factor + scale_factor;
    }
    
    if (marquee_end) {
        display_draw_char(x, y, scale_factor, COLOR_WHITE, '>');
    }
}

void render_g_text_box(g_text_box_t* inst) {
    if (!inst->enabled) return;
    if (inst->text == NULL || inst->length == 0) return;


    coord_t chars_per_line = g_text_box_chars_per_line(inst);
    coord_t line_number = 1;
    coord_t y = inst->y1 + FONT_HEIGHT * inst->scale_factor;
    bool render_line;

    size_t marquee_index = 0;
    if (inst->truncation_mode == TEXT_MARQUEE) {
        inst->_marquee.enabled = chars_per_line < inst->length;
        if (inst->_marquee.enabled) {
            marquee_index = inst->_marquee.index;
        }
    }

    size_t line_start_i = marquee_index;
    size_t i;
    for (i = line_start_i; i < inst->length + 1; i++) {
        render_line =
            i == inst->length ||        // end of string
            (inst->truncation_mode != TEXT_MARQUEE && inst->text[i] == '\n') ||         // LF (force marquees to single line)
            (inst->truncation_mode == TEXT_WRAP && i - line_start_i >= chars_per_line); // text wrap
    
        if (!render_line) continue;
    

        if (marquee_index == 0 && i - line_start_i <= chars_per_line) {
            // the line fits
            graphics_render_text_line_internal(
                inst->x1, y, inst->x2, inst->alignment_mode, inst->scale_factor, 
                inst->color, &inst->text[line_start_i], i - line_start_i, false, false);
        } else {
            // the line is truncated
            switch (inst->truncation_mode) {
                case TEXT_MARQUEE:
                    graphics_render_text_line_internal(
                        inst->x1, y, inst->x2, inst->alignment_mode, inst->scale_factor, 
                        inst->color, &inst->text[line_start_i], 
                        chars_per_line,
                        marquee_index > 0, 
                        i - line_start_i > chars_per_line);
                    break;

                case TEXT_TRUNCATE:
                default:
                    graphics_render_text_line_internal(
                        inst->x1, y, inst->x2, inst->alignment_mode, inst->scale_factor, 
                        inst->color, &inst->text[line_start_i], chars_per_line, false, false);
                    break;
            }
        }

        line_start_i = i + (inst->text[i] == '\n' ? 1 : 0);
        y += FONT_HEIGHT * inst->scale_factor + inst->line_spacing;
        line_number++;
        if (inst->max_lines != 0 && line_number > inst->max_lines) break;
    }

}

void render_g_rectangle(g_rectangle_t* inst) {
    if (!inst->enabled) return;
    if (inst->filled) {
        display_draw_rectangle(inst->x1, inst->y1, inst->x2, inst->y2, inst->color);
    } else {
        display_draw_rectangle_outline(inst->x1, inst->y1, inst->x2, inst->y2, inst->color);
    }
}

void render_g_line(g_line_t* inst) {
    if (!inst->enabled) return;
    display_draw_line(inst->x1, inst->y1, inst->x2, inst->y2, inst->color);
}

void graphics_add_object(g_object_holder_t holder) {
    if (graphics_object_count >= GRAPHICS_MAX_OBJECTS) {
        printf("ERROR: can't add any more graphics objects\n");
        return;
    }
    graphics_objects_internal[graphics_object_count++] = holder;
}


coord_t g_text_box_height(g_text_box_t* inst) {
    if (inst->text == NULL || inst->length == 0) return 0;
    if (inst->max_lines == 1 || inst->truncation_mode == TEXT_MARQUEE) return FONT_HEIGHT * inst->scale_factor;

    coord_t max_line_len = inst->x2 - inst->x1 + 1;
    coord_t cur_line_len = 0;
    coord_t height = FONT_HEIGHT * inst->scale_factor;
    coord_t line_number = 1;
    char c;
    bool newline;

    // loop to properly handle LF chars
    for (size_t i = 0; i < inst->length; i++) {
        c = inst->text[i];

        if (c == '\n') {
            newline = true;
        } else {
            cur_line_len += FONT_WIDTH * inst->scale_factor + inst->scale_factor;
            newline = inst->truncation_mode == TEXT_WRAP && cur_line_len + FONT_WIDTH * inst->scale_factor > max_line_len;
        }

        if (newline) {
            line_number++;
            if (inst->max_lines != 0 && line_number > inst->max_lines) break;

            height += FONT_HEIGHT * inst->scale_factor + inst->line_spacing;
            cur_line_len = 0;
        }
    }
    
    return height;
}

void g_text_box_print_internal(g_text_box_t* inst, char* text, size_t length) {
    if (inst->text != NULL && inst->length != 0) free(inst->text);
    inst->text = text;
    inst->length = length;
}

void g_text_box_print(g_text_box_t* inst, char* text) {
    size_t length = strlen(text);
    char* text_ptr = malloc(length);

    for (size_t i = 0; i < length; i++) 
        text_ptr[i] = text[i];

    g_text_box_print_internal(inst, text_ptr, length);
}

void g_text_box_printf(g_text_box_t* inst, char* text, ...) {
    va_list args;
    va_start(args, text);

    // this is terrible, but I don't like wasting ram.
    size_t length = vsprintf(NULL, text, args);
    char* text_ptr = malloc(length);
    vsprintf(text_ptr, text, args);

    va_end(args);

    g_text_box_print_internal(inst, text_ptr, length);
}

g_text_box_t* get_g_text_box_inst() {
    if (graphics_text_box_alloc_index == GRAPHICS_MAX_TEXT_BOXES) {
        // this should be a panic, but crashing would be sub-optimal
        printf("FATAL: not enough text box instances!!!!\n");
        return &graphics_text_box_pool[graphics_text_box_alloc_index - 1];
    }

    g_text_box_t* inst = &graphics_text_box_pool[graphics_text_box_alloc_index++];
    init_g_text_box(inst);
    return inst;
}

g_rectangle_t* get_g_rectangle_inst() {
    if (graphics_rectangle_alloc_index == GRAPHICS_MAX_RECTS) {
        // this should be a panic, but crashing would be sub-optimal
        printf("FATAL: not enough rectangle instances!!!!\n");
        return &graphics_rectangle_pool[graphics_rectangle_alloc_index - 1];
    }

    g_rectangle_t* inst = &graphics_rectangle_pool[graphics_rectangle_alloc_index++];
    init_g_rectangle(inst);
    return inst;
}

g_line_t* get_g_line_inst() {
    if (graphics_line_alloc_index == GRAPHICS_MAX_LINES) {
        // this should be a panic, but crashing would be sub-optimal
        printf("FATAL: not enough line instances!!!!\n");
        return &graphics_line_pool[graphics_line_alloc_index - 1];
    }

    g_line_t* inst = &graphics_line_pool[graphics_line_alloc_index++];
    init_g_line(inst);
    return inst;
}

void setup_g_text_box(g_text_box_t* inst, coord_t x1, coord_t y1, coord_t x2, coord_t max_lines, color_t color) {
    inst->x1 = x1;
    inst->y1 = y1;
    inst->x2 = x2;
    inst->max_lines = max_lines;
    inst->color = color;
}

void setup_g_rectangle(g_rectangle_t* inst, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color, bool filled) {
    inst->x1 = x1;
    inst->y1 = y1;
    inst->x2 = x2;
    inst->y2 = y2;
    inst->color = color;
    inst->filled = filled;
}

void setup_g_line(g_line_t* inst, coord_t x1, coord_t y1, coord_t x2, coord_t y2, color_t color) {
    inst->x1 = x1;
    inst->y1 = y1;
    inst->x2 = x2;
    inst->y2 = y2;
    inst->color = color;
}

void graphics_add_text_box(g_text_box_t* inst) {
    g_object_ptr_t ptr;
    ptr.text_box = inst;
    g_object_holder_t holder = {
        type: GRAPHICS_TEXT_BOX,
        ptr: ptr,
    };
    graphics_add_object(holder);
}

void graphics_add_rectangle(g_rectangle_t* inst) {
    g_object_ptr_t ptr;
    ptr.rectangle = inst;
    g_object_holder_t holder = {
        type: GRAPHICS_RECTANGLE,
        ptr: ptr,
    };
    graphics_add_object(holder);
}

void graphics_add_line(g_line_t* inst) {
    g_object_ptr_t ptr;
    ptr.line = inst;
    g_object_holder_t holder = {
        type: GRAPHICS_LINE,
        ptr: ptr,
    };
    graphics_add_object(holder);
}

void graphics_render() {
    g_object_holder_t* holder;

    display_clear();

    for (size_t i = 0; i < graphics_object_count; i++) {
        holder = &graphics_objects_internal[i];
        switch (holder->type) {
            case GRAPHICS_TEXT_BOX:
                render_g_text_box(holder->ptr.text_box);
                break;
            case GRAPHICS_RECTANGLE:
                render_g_rectangle(holder->ptr.rectangle);
                break;
            case GRAPHICS_LINE:
                render_g_line(holder->ptr.line);
                break;
        }
    }
    
    display_refresh();
}

void graphics_update() {
    g_object_holder_t* holder;
    g_text_box_t* text_box;
    uint64_t timestamp = time_us_64();
    bool marquee_updates = false;

    // update marquees
    for (size_t i = 0; i < graphics_object_count; i++) {
        holder = &graphics_objects_internal[i];
        if (holder->type != GRAPHICS_TEXT_BOX) continue;

        text_box = holder->ptr.text_box;
        if (!text_box->_marquee.enabled || text_box->truncation_mode != TEXT_MARQUEE) continue;

        // init timestamp
        if (text_box->_marquee.last_updated == 0) {
            text_box->_marquee.last_updated = timestamp;
            continue;
        }
        
        // handle delays
        if (text_box->_marquee.index == 0) {
            if (text_box->_marquee.last_updated + MARQUEE_START_DELAY > timestamp) continue;
        } else {
            if (text_box->_marquee.last_updated + MARQUEE_INTERVAL > timestamp) continue;
        }
        
        text_box->_marquee.last_updated = timestamp;
        marquee_updates = true;
        
        text_box->_marquee.index++;
        if (text_box->_marquee.index >= text_box->length) {
            text_box->_marquee.index = 0;
        }

    }

    if (marquee_updates) graphics_render();
}

void graphics_reset() {
    // free strings from the heap
    for (size_t i = 0; i < graphics_text_box_alloc_index; i++) {
        init_g_text_box(&graphics_text_box_pool[i]);
    }
    
    graphics_text_box_alloc_index = 0;
    graphics_rectangle_alloc_index = 0;
    graphics_line_alloc_index = 0;

    graphics_object_count = 0;
}

void init_graphics() {
    graphics_reset();
}


