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
#include <stdint.h>
#include <x68k/iocs.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "modx68k.h"

/****************************************************************************/

bool x68k_super_mode = false;
STATIC int super_ssp;

STATIC bool to_super(bool mode) {
    if (mode && !x68k_super_mode) {
        super_ssp = _iocs_b_super(0);
        if (super_ssp < 0) {
            super_ssp = 0;
        }
    } else if (!mode && x68k_super_mode) {
        if (super_ssp != 0) {
            _iocs_b_super(super_ssp);
            super_ssp = 0;
        }
    }
    int res = x68k_super_mode;
    x68k_super_mode = mode;
    return res;
}

typedef struct _mp_obj_x68k_super_t {
    mp_obj_base_t base;
    bool oldstat;
} mp_obj_x68k_super_t;


STATIC mp_obj_t x68k_super_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    mp_obj_x68k_super_t *self = mp_obj_malloc(mp_obj_x68k_super_t, type);
    self->oldstat = to_super(true);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t x68k_super___exit__(size_t n_args, const mp_obj_t *args) {
    mp_obj_x68k_super_t *self = MP_OBJ_TO_PTR(args[0]);
    to_super(self->oldstat);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_super___exit___obj, 4, 4, x68k_super___exit__);

STATIC const mp_rom_map_elem_t x68k_super_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&x68k_super___exit___obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_super_locals_dict, x68k_super_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_super,
    MP_QSTR_Super,
    MP_TYPE_FLAG_NONE,
    make_new, x68k_super_make_new,
    locals_dict, &x68k_super_locals_dict
    );

STATIC mp_obj_t x68k_super(size_t n_args, const mp_obj_t *args) {
    bool mode = true;
    if (n_args > 0) {
        mode = mp_obj_is_true(args[0]);
    }
    return to_super(mode) ? mp_const_true : mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_super_obj, 0, 1, x68k_super);

STATIC mp_obj_t x68k_issuper(void) {
    return x68k_super_mode ? mp_const_true : mp_const_false;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(x68k_issuper_obj, x68k_issuper);

/****************************************************************************/

STATIC mp_obj_t x68k_mpyaddr(void) {
    extern char _start;
    return MP_OBJ_NEW_SMALL_INT((int)&_start);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(x68k_mpyaddr_obj, x68k_mpyaddr);

/****************************************************************************/

STATIC mp_obj_t x68k_crtmod(size_t n_args, const mp_obj_t *args) {
    mp_int_t mode = mp_obj_get_int(args[0]);
    bool clron = false;

    if (n_args > 1) {
        clron = mp_obj_get_int(args[1]);
    }
    _iocs_crtmod(mode);
    if (clron) {
        _iocs_g_clr_on();
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_crtmod_obj, 1, 2, x68k_crtmod);

STATIC mp_obj_t x68k_curon(void) {
    _iocs_os_curon();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(x68k_curon_obj, x68k_curon);

STATIC mp_obj_t x68k_curoff(void) {
    _iocs_os_curof();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(x68k_curoff_obj, x68k_curoff);

#define REG_GPIP        (0xE88001)

STATIC mp_obj_t x68k_vsync(void) {
    int oldstat = to_super(true);
    while ((*(volatile uint8_t *)REG_GPIP & 0x10) == 0) {
        MICROPY_EVENT_POLL_HOOK
    }
    while ((*(volatile uint8_t *)REG_GPIP & 0x10) != 0) {
        MICROPY_EVENT_POLL_HOOK
    }
    to_super(oldstat);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(x68k_vsync_obj, x68k_vsync);

/****************************************************************************/

STATIC mp_obj_t x68k_fontrom(void) {
    return mp_obj_new_memoryview('B', 0xc0000, (void *)0xf00000);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(x68k_fontrom_obj, x68k_fontrom);

/****************************************************************************/

STATIC const mp_rom_map_elem_t mp_module_x68k_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_x68k) },

    { MP_ROM_QSTR(MP_QSTR_Super), MP_ROM_PTR(&x68k_type_super) },
    { MP_ROM_QSTR(MP_QSTR_super), MP_ROM_PTR(&x68k_super_obj) },
    { MP_ROM_QSTR(MP_QSTR_issuper), MP_ROM_PTR(&x68k_issuper_obj) },

    { MP_ROM_QSTR(MP_QSTR_IntOpm), MP_ROM_PTR(&x68k_type_intopm) },
    { MP_ROM_QSTR(MP_QSTR_IntTimerD), MP_ROM_PTR(&x68k_type_inttimerd) },
    { MP_ROM_QSTR(MP_QSTR_IntVSync), MP_ROM_PTR(&x68k_type_intvsync) },
    { MP_ROM_QSTR(MP_QSTR_IntRaster), MP_ROM_PTR(&x68k_type_intraster) },
    { MP_ROM_QSTR(MP_QSTR_IntDisable), MP_ROM_PTR(&x68k_type_intdisable) },
    { MP_ROM_QSTR(MP_QSTR_intDisable), MP_ROM_PTR(&x68k_intdisable_obj) },
    { MP_ROM_QSTR(MP_QSTR_intEnable), MP_ROM_PTR(&x68k_intenable_obj) },

    { MP_ROM_QSTR(MP_QSTR_mpyaddr), MP_ROM_PTR(&x68k_mpyaddr_obj) },

    { MP_ROM_QSTR(MP_QSTR_iocs), MP_ROM_PTR(&x68k_iocs_obj) },
    { MP_ROM_QSTR(MP_QSTR_i), MP_ROM_PTR(&x68k_i_obj_type) },
    { MP_ROM_QSTR(MP_QSTR_dos), MP_ROM_PTR(&x68k_dos_obj) },
    { MP_ROM_QSTR(MP_QSTR_d), MP_ROM_PTR(&x68k_d_obj_type) },

    { MP_ROM_QSTR(MP_QSTR_crtmod), MP_ROM_PTR(&x68k_crtmod_obj) },
    { MP_ROM_QSTR(MP_QSTR_vsync), MP_ROM_PTR(&x68k_vsync_obj) },
    { MP_ROM_QSTR(MP_QSTR_curon), MP_ROM_PTR(&x68k_curon_obj) },
    { MP_ROM_QSTR(MP_QSTR_curoff), MP_ROM_PTR(&x68k_curoff_obj) },
    { MP_ROM_QSTR(MP_QSTR_fontrom), MP_ROM_PTR(&x68k_fontrom_obj) },

    { MP_ROM_QSTR(MP_QSTR_vpage), MP_ROM_PTR(&x68k_vpage_obj) },
    { MP_ROM_QSTR(MP_QSTR_GVRam), MP_ROM_PTR(&x68k_type_gvram) },
    { MP_ROM_QSTR(MP_QSTR_TVRam), MP_ROM_PTR(&x68k_type_tvram) },
    { MP_ROM_QSTR(MP_QSTR_Sprite), MP_ROM_PTR(&x68k_type_sprite) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_x68k_globals, mp_module_x68k_globals_table);

const mp_obj_module_t mp_module_x68k = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*) &mp_module_x68k_globals,
};

MP_REGISTER_MODULE(MP_QSTR_x68k, mp_module_x68k);
