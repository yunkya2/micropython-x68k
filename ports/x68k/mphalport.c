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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <x68k/dos.h>
#include <x68k/iocs.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mpconfig.h"
#include "py/misc.h"

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    unsigned char c = 0;
    c = _dos_inkey();
//    printf("[%02x]", c); fflush(stdout);

    return c;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    int r = write(STDOUT_FILENO, str, len);
}

mp_uint_t mp_hal_ticks_ms(void) {
    struct iocs_time t;
    t = _iocs_ontime();
    return t.sec * 10;
}

mp_uint_t mp_hal_ticks_us(void) {
    return mp_hal_ticks_ms() * 1000;
}

mp_uint_t mp_hal_ticks_cpu(void) {
    return 0;
}

uint64_t mp_hal_time_ns(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000000ULL +
           (uint64_t)tv.tv_usec * 1000ULL;
}

void mp_hal_delay_ms(mp_uint_t ms) {
    struct iocs_time t1, t2;
    t1 = _iocs_ontime();
    while (1) {
        t2 = _iocs_ontime();
        if (t2.sec - t1.sec >= ms / 10)
            break;
        MICROPY_EVENT_POLL_HOOK
    }
}

void mp_hal_delay_us(mp_uint_t us) {
    mp_hal_delay_ms(us / 1000);
}

void mp_hal_set_interrupt_char(char c) {
    if (c == 3) {
        _dos_breakck(1);
    } else {
        _dos_breakck(2);
    }
}

STATIC struct {
    int fno;
    const char *keydef;
} setfnckey[] =
{
    24, "\x04",     /* DEL   -- ^D */
    25, "\x10",     /* UP    -- ^P */
    26, "\x02",     /* LEFT  -- ^B */
    27, "\x06",     /* RIGHT -- ^F */
    28, "\x0e",     /* DOWN  -- ^N */
    29, "\x15",     /* CLR   -- ^U */
    31, "\x01",     /* HOME  -- ^A */
};

STATIC char savefnckey[MP_ARRAY_SIZE(setfnckey)][6];

void mp_hal_setfnckey(void) {
    int i;
    for (i = 0; i < MP_ARRAY_SIZE(setfnckey); i++) {
        _dos_fnckeygt(setfnckey[i].fno, savefnckey[i]);
        _dos_fnckeyst(setfnckey[i].fno, setfnckey[i].keydef);
    }
}

void mp_hal_restorefnckey(void) {
    int i;
    for (i = 0; i < MP_ARRAY_SIZE(setfnckey); i++) {
        _dos_fnckeyst(setfnckey[i].fno, savefnckey[i]);
    }
}
