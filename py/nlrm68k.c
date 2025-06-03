/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Yuichi Nakamura
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

#include "py/mpstate.h"

#if MICROPY_NLR_M68K

__asm(
    ".globl  nlr_push\n"
    "nlr_push:\n"
    "moveal %sp@(4),%a0\n"    // Load nlr_buf
    "movel  %sp,%a0@(8)\n"    // Store SP into nlr_buf
    "movel  %sp@,%a0@(12)\n"  // Store return address into nlr_buf
    "moveml %d2-%d7/%a2-%a6,%a0@(16)\n" // Store callee-saved registers
    "bra    nlr_push_tail\n"    // Jump to the C part
    );

NORETURN void nlr_jump(void *val) {
    MP_NLR_JUMP_HEAD(val, top)
    __asm volatile (
        "moveal %0, %%a0\n"         // a0 points to nlr_buf"
        "moveml %%a0@(16),%%d2-%%d7/%%a2-%%a6\n" // Retrieve callee-saved registers
        "moveal %%a0@(8),%%sp\n"    // Retrieve SP.
        "movel  %%a0@(12),%%sp@\n"  // Retrieve return address
        "moveql #1,%%d0\n"          // Return 1, non-local return.
        "rts\n"
        :                      // Outputs.
        : "r" (top)            // Inputs.
        : "memory"             // Clobbered.
        );

    MP_UNREACHABLE
}

#endif // MICROPY_NLR_M68K
