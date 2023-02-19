/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "py/mpconfig.h"

// wrapper around everything in this file
#if MICROPY_EMIT_M68K || MICROPY_EMIT_INLINE_M68K

#include "py/mpstate.h"
#include "py/asmm68k.h"

static inline byte *asm_m68k_get_cur_to_write_bytes(asm_m68k_t *as, int n) {
    return mp_asm_base_get_cur_to_write_bytes(&as->base, n);
}

void asm_m68k_entry(asm_m68k_t *as, int num_locals) {
    assert(num_locals >= 0);
    asm_m68k_op16(as, 0x4e56);  //  link    a6,#0
    asm_m68k_op16(as, 0x0000);
    asm_m68k_op16(as, 0x48e7);  //  movem.l d2-d7/a2-a5,-(sp)
    asm_m68k_op16(as, 0x3f3c);
}

void asm_m68k_exit(asm_m68k_t *as) {
    asm_m68k_op16(as, 0x4cdf);  //  movem.l (sp)+,d2-d7/a2-a5
    asm_m68k_op16(as, 0x3cfc);
    asm_m68k_op16(as, 0x4e5e);  //  unlk    a6
    asm_m68k_op16(as, 0x4e75);  //  rts
}

void asm_m68k_op16(asm_m68k_t *as, uint op) {
    byte *c = asm_m68k_get_cur_to_write_bytes(as, 2);
    if (c != NULL) {
        // big endian
        c[0] = op >> 8;
        c[1] = op;
#ifdef DEBUG
        printf("emit: %04x\n", op & 0xffff);
#endif
    }
}

void asm_m68k_op32(asm_m68k_t *as, uint32_t op) {
    byte *c = asm_m68k_get_cur_to_write_bytes(as, 4);
    if (c != NULL) {
        // big endian
        c[0] = op >> 24;
        c[1] = op >> 16;
        c[2] = op >> 8;
        c[3] = op;
#ifdef DEBUG
        printf("emit: %08x\n", op);
#endif
    }
}

#endif // MICROPY_EMIT_M68K || MICROPY_EMIT_INLINE_M68K
