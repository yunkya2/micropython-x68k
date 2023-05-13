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

// Set base feature level.
#define MICROPY_CONFIG_ROM_LEVEL (MICROPY_CONFIG_ROM_LEVEL_EXTRA_FEATURES)

// Board and hardware specific configuration
#define MICROPY_HW_MCU_NAME            "m68000"
#define MICROPY_HW_BOARD_NAME          "x68k"
#define MICROPY_PY_SYS_PLATFORM        "human68k"

// Enable helpers for printing debugging information.
#ifndef MICROPY_DEBUG_PRINTERS
#define MICROPY_DEBUG_PRINTERS         (1)
#endif

// Enable floating point by default.
#ifndef MICROPY_FLOAT_IMPL
#define MICROPY_FLOAT_IMPL             (MICROPY_FLOAT_IMPL_FLOAT)
#endif

// Enable arbritrary precision long-int by default.
#ifndef MICROPY_LONGINT_IMPL
#define MICROPY_LONGINT_IMPL           (MICROPY_LONGINT_IMPL_MPZ)
#endif

// REPL conveniences.
#define MICROPY_REPL_EMACS_WORDS_MOVE  (1)
#define MICROPY_REPL_EMACS_EXTRA_WORDS_MOVE (1)
#define MICROPY_USE_READLINE_HISTORY   (1)
#ifndef MICROPY_READLINE_HISTORY_SIZE
#define MICROPY_READLINE_HISTORY_SIZE  (50)
#endif

// Allow exception details in low-memory conditions.
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE (0)

// Allow loading of .mpy files.
#define MICROPY_PERSISTENT_CODE_LOAD   (1)

// Enable a small performance boost for the VM.
#define MICROPY_OPT_COMPUTED_GOTO      (1)

// Enable detailed error messages.
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_DETAILED)

// Configure the "sys" module with features not usually enabled on bare-metal.
#define MICROPY_PY_SYS_ATEXIT          (1)

// Configure the "os" module with extra unix features.
#define MICROPY_PY_UOS_INCLUDEFILE     "ports/x68k/moduos.c"
#define MICROPY_PY_UOS_ERRNO           (1)
#define MICROPY_PY_UOS_GETENV_PUTENV_UNSETENV (1)
#define MICROPY_PY_UOS_SEP             (1)
#define MICROPY_PY_UOS_SYSTEM          (1)
#define MICROPY_PY_UOS_UNAME           (1)
#define MICROPY_PY_UOS_STATVFS         (1)
#define MICROPY_PY_UOS_URANDOM         (0)
#define MICROPY_PY_UOS_SYNC            (0)

// Enable the unix-specific "time" module.
#define MICROPY_PY_UTIME               (1)
#define MICROPY_PY_UTIME_MP_HAL        (1)

// Disable the "select" module.
#define MICROPY_PY_USELECT             (0)

// Enable the "machine" module.
#define MICROPY_PY_MACHINE             (1)
#define MICROPY_PY_MACHINE_PIN_MAKE_NEW     mp_pin_make_new

#ifndef MICROPY_PY_SYS_PATH_DEFAULT
#define MICROPY_PY_SYS_PATH_DEFAULT ".frozen"
#endif

#define MP_STATE_PORT MP_STATE_VM

// Configure which emitter to use for this target.
#define MICROPY_EMIT_INLINE_M68K    (1)
#define MICROPY_EMIT_M68K           (1)

// Type definitions for the specific machine based on the word size.
typedef intptr_t mp_int_t; // must be pointer size
typedef uintptr_t mp_uint_t; // must be pointer size

// Cannot include <sys/types.h>, as it may lead to symbol name clashes
typedef long mp_off_t;

#define MICROPY_OBJ_BASE_ALIGNMENT  __attribute__((aligned(4)))

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

// Always enable GC.
#define MICROPY_ENABLE_GC           (1)

#if !(defined(MICROPY_GCREGS_SETJMP))
// Fall back to setjmp() implementation for discovery of GC pointers in registers.
#define MICROPY_GCREGS_SETJMP (1)
#endif

// Enable the VFS
#define MICROPY_ENABLE_FINALISER    (1)
#define MICROPY_VFS                 (1)
#define MICROPY_READER_VFS          (1)
#define MICROPY_VFS_POSIX           (0)

// VFS stat functions should return time values relative to 1970/1/1
#define MICROPY_EPOCH_IS_1970       (1)

// Disable stackless by default.
#ifndef MICROPY_STACKLESS
#define MICROPY_STACKLESS           (0)
#define MICROPY_STACKLESS_STRICT    (0)
#endif

// Ensure builtinimport.c works with -m.
#define MICROPY_MODULE_OVERRIDE_MAIN_IMPORT (1)

// Don't default sys.argv because we do that in main.
#define MICROPY_PY_SYS_PATH_ARGV_DEFAULTS (0)

// Bare-metal ports don't have stderr. Printing debug to stderr may give tests
// which check stdout a chance to pass, etc.
extern const struct _mp_print_t mp_stderr_print;
#define MICROPY_DEBUG_PRINTER (&mp_stderr_print)
#define MICROPY_ERROR_PRINTER (&mp_stderr_print)

#ifndef MICROPY_EVENT_POLL_HOOK
#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(true); \
    } while (0);
#endif

// Python internal features.
#define MICROPY_SMALL_INT_MUL_HELPER            (1)

#define MICROPY_SCHEDULER_STATIC_NODES          (1)

#define MICROPY_PY_USELECT                      (0)
#define MICROPY_PY_ASYNC_AWAIT                  (0)
#define MICROPY_PY_UASYNCIO                     (0)

#define MICROPY_PY_BUILTINS_STR_UNICODE         (0)
#define MICROPY_PY_BUILTINS_STR_SJIS            (1)
#define MICROPY_PY_BUILTINS_STR_SJIS_CHECK      (1)

#define MICROPY_PY_URE                          (1)
#define MICROPY_PY_URE_MATCH_GROUPS             (1)
#define MICROPY_PY_URE_MATCH_SPAN_START_END     (1)
#define MICROPY_PY_URE_SUB                      (1)

#define MP_SSIZE_MAX (0x7fffffff)
