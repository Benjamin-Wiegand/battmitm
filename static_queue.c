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
#include "static_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// statically allocated queue
// it supports concurrently to an extent, but only if each core/interrupt only ever adds or removes from it.

static_queue_t* create_static_queue(size_t max_elements, size_t element_size) {
    void* data = malloc(max_elements * element_size);
    if (data == NULL) {
        printf("FATAL: static queue allocation failed\n");
        return NULL;
    }

    static_queue_t* inst = malloc(sizeof(static_queue_t));
    inst->max_elements = max_elements;
    inst->element_size = element_size;
    inst->dropped = 0;
    inst->_index_end = 0;
    inst->_index_start = 0;
    inst->_data = data;
    return inst;
}

void increment_index(static_queue_t* inst, size_t* index) {
    if (*index >= inst->max_elements - 1) {
        *index = 0;
    } else {
        (*index)++;
    }
}

bool static_queue_is_full(static_queue_t* inst) {
    return inst->_index_start == inst->_index_end + 1 || (inst->_index_end == inst->max_elements - 1 && inst->_index_start == 0);
}

bool static_queue_is_empty(static_queue_t* inst) {
    return inst->_index_start == inst->_index_end;
}

void* static_queue_get_element_ptr(static_queue_t* inst, size_t index) {
    return inst->_data + index * inst->element_size;
}

// returns a pointer to place data for your new queue element.
// if the queue is full this returns a NULL pointer and increments `dropped`.
// note that this will immediately make this memory available to be accessed (peeked/popped) from the queue.
// if queue access may interrupt this consider adding an "initialized" byte to the element allocation
void* static_queue_add(static_queue_t* inst) {
    if (static_queue_is_full(inst)) {
        inst->dropped++;
        return NULL;
    }
    
    void* ptr = static_queue_get_element_ptr(inst, inst->_index_end);
    increment_index(inst, &inst->_index_end);

    return ptr;
}

// removes an element off the front of the queue and returns a pointer to it.
// returns a NULL pointer if the queue is empty.
// note that this immediately opens that memory to get claimed by the next addition to the queue.
// if queue additions may interrupt this consider using peek instead, then call pop when you're done.
void* static_queue_pop(static_queue_t* inst) {
    if (static_queue_is_empty(inst)) {
        return NULL;
    }

    void* ptr = static_queue_get_element_ptr(inst, inst->_index_start);
    increment_index(inst, &inst->_index_start);

    return ptr;
}

// returns a pointer to the element at the front of the queue.
void* static_queue_peek(static_queue_t* inst) {
    if (static_queue_is_empty(inst)) {
        return NULL;
    }

    return static_queue_get_element_ptr(inst, inst->_index_start);
}

// returns the number of elements inside of the queue
size_t static_queue_size(static_queue_t* inst) {
    if (inst->_index_start < inst->_index_end) {
        return inst->_index_end - inst->_index_start;
    } else if (inst->_index_start > inst->_index_end) {
        return inst->max_elements - inst->_index_start + inst->_index_end;
    } else {
        return 0; // empty if both are the same
    }
}


