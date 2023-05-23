/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2014 Paul Sokolovsky
 * Copyright (c) 2023 Yuichi Nakamura
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef MICROPY_INCLUDED_MODX68KXARRAY_H
#define MICROPY_INCLUDED_MODX68KXARRAY_H

#include "py/obj.h"

// 1-dimension xarray header
typedef struct _x68k_xarray_head_t {
    size_t size;        // total data size within header
    uint16_t dim;       // # of dimension - 1
    uint16_t unitsz;    // data unit size
    uint16_t sub1;      // 1st dimension subscript
} x68k_xarray_head_t;

// 2-dimension xarray header
typedef struct _x68k_xarray2_head_t {
    x68k_xarray_head_t dim1;
    uint16_t sub2;      // 2nd dimension subscript
    size_t dim1sz;      // 1st dimension data size
} x68k_xarray2_head_t;

typedef struct _mp_obj_x68k_xarray_t {
    mp_obj_base_t base;
    int typecode;
    size_t len;         // in elements
    void *items;        // direct pointer to the xarray contents
    int dim;            // # of dimension
    x68k_xarray2_head_t *head;  // pointer to the xarray header
} mp_obj_x68k_xarray_t;

// Accessor macros
#define xarray_dim(o)       ((o)->dim)
#define xarray_sub1(o)      ((o)->head->dim1.sub1 + 1)
#define xarray_sub2(o)      ((o)->head->sub2 + 1)

#endif // MICROPY_INCLUDED_MODX68KXARRAY_H
