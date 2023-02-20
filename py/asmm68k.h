/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) Yuichi Nakamura
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
#ifndef MICROPY_INCLUDED_PY_ASMM68K_H
#define MICROPY_INCLUDED_PY_ASMM68K_H

#include <assert.h>
#include "py/misc.h"
#include "py/asmbase.h"
#include "py/persistentcode.h"

typedef struct _asm_m68k_t {
    mp_asm_base_t base;
} asm_m68k_t;

static inline void asm_m68k_end_pass(asm_m68k_t *as) {
    (void)as;
}

void asm_m68k_entry(asm_m68k_t *as, int num_locals);
void asm_m68k_exit(asm_m68k_t *as);

void asm_m68k_op16(asm_m68k_t *as, uint op);
void asm_m68k_op32(asm_m68k_t *as, uint32_t op);

#endif // MICROPY_INCLUDED_PY_ASMM68K_H
