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

STATIC void to_super(void)
{
    static int super;
    if (!super) {
        _iocs_b_super(0);
        super = 1;
    }
}

STATIC mp_obj_t mp_x68k_crtmod(size_t n_args, const mp_obj_t *args) {
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
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_x68k_crtmod_obj, 1, 2, mp_x68k_crtmod);

STATIC mp_obj_t mp_x68k_gvram(void) {
    to_super();
    return mp_obj_new_memoryview('B'|MP_OBJ_ARRAY_TYPECODE_FLAG_RW,
                                 0x200000, (void *)0xc00000);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_x68k_gvram_obj, mp_x68k_gvram);

STATIC mp_obj_t mp_x68k_tvram(void) {
    to_super();
    return mp_obj_new_memoryview('B'|MP_OBJ_ARRAY_TYPECODE_FLAG_RW,
                                 0x80000, (void *)0xe00000);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_x68k_tvram_obj, mp_x68k_tvram);

STATIC mp_obj_t mp_x68k_fontrom(void) {
    to_super();
    return mp_obj_new_memoryview('B', 0xc0000, (void *)0xf00000);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mp_x68k_fontrom_obj, mp_x68k_fontrom);

STATIC const mp_rom_map_elem_t mp_module_x68k_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_x68k) },

    { MP_ROM_QSTR(MP_QSTR_crtmod), MP_ROM_PTR(&mp_x68k_crtmod_obj) },
    { MP_ROM_QSTR(MP_QSTR_gvram), MP_ROM_PTR(&mp_x68k_gvram_obj) },
    { MP_ROM_QSTR(MP_QSTR_tvram), MP_ROM_PTR(&mp_x68k_tvram_obj) },
    { MP_ROM_QSTR(MP_QSTR_fontrom), MP_ROM_PTR(&mp_x68k_fontrom_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_x68k_globals, mp_module_x68k_globals_table);

const mp_obj_module_t mp_module_x68k = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*) &mp_module_x68k_globals,
};

MP_REGISTER_MODULE(MP_QSTR_x68k, mp_module_x68k);
