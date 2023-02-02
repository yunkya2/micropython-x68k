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

// Options controlling how MicroPython is built, overriding defaults in py/mpconfig.h

#include <stdint.h>

// Board and hardware specific configuration
#define MICROPY_HW_MCU_NAME                     "m68000"
#define MICROPY_HW_BOARD_NAME                   "x68k"

#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES)

#define MICROPY_OBJ_BASE_ALIGNMENT  __attribute__((aligned(4)))

// Python internal features.
#define MICROPY_ENABLE_GC                       (1)
#define MICROPY_GCREGS_SETJMP                   (1)
#define MICROPY_HELPER_REPL                     (1)
#define MICROPY_ERROR_REPORTING                 (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_FLOAT_IMPL                      (MICROPY_FLOAT_IMPL_FLOAT)

#define MICROPY_MODULE_WEAK_LINKS               (1)
#define MICROPY_CAN_OVERRIDE_BUILTINS           (0)
#define MICROPY_PY_FUNCTION_ATTRS               (1)

#define MICROPY_PY_ASYNC_AWAIT                  (0)
#define MICROPY_PY_BUILTINS_SET                 (0)
#define MICROPY_PY_ATTRTUPLE                    (0)
#define MICROPY_PY_COLLECTIONS                  (0)
#define MICROPY_PY_MATH                         (1)
#define MICROPY_PY_IO                           (0)
#define MICROPY_PY_STRUCT                       (1)
#define MICROPY_PY_MACHINE                      (1)
#define MICROPY_VFS                             (0)

#define MICROPY_PY_UOS                          (0)
#define MICROPY_PY_UOS_INCLUDEFILE  "ports/x68k/moduos.c"
#define MICROPY_PY_UOS_ERRNO                    (1)
#define MICROPY_PY_UOS_SEP                      (1)
#define MICROPY_PY_UOS_SYSTEM                   (0)
#define MICROPY_PY_SYS_STDFILES                 (0)

#define MICROPY_PY_UJSON                        (0)

#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(true); \
        /* mp_hal_delay_us(500); */ \
    } while (0);

#define MICROPY_REPL_EMACS_WORDS_MOVE           (1)
#define MICROPY_REPL_EMACS_EXTRA_WORDS_MOVE     (1)
#define MICROPY_USE_READLINE_HISTORY            (1)
#ifndef MICROPY_READLINE_HISTORY_SIZE
#define MICROPY_READLINE_HISTORY_SIZE           (50)
#endif

#define MICROPY_PORT_ROOT_POINTERS                                        \
    const char *readline_hist[8];                                         \

#define MP_STATE_PORT MP_STATE_VM

// Type definitions for the specific machine.

typedef intptr_t mp_int_t; // must be pointer size
typedef uintptr_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// We need to provide a declaration/definition of alloca()
#include <alloca.h>
