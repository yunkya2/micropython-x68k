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

STATIC mp_obj_t mp_x68k_iocs(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_d0, ARG_d1, ARG_d2, ARG_d3, ARG_d4, ARG_d5, 
           ARG_a1, ARG_a2, ARG_a1w, ARG_a2w, ARG_rd, ARG_ra };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_d0,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_d1,   MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_d2,   MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_d3,   MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_d4,   MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_d5,   MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_a1,   MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_a2,   MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_a1w,  MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_a2w,  MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rd,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_ra,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                    MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_int_t regs[8] = {0};
    mp_int_t rd = 0, ra = 0;
    int i;

    for (i = 0; i <= ARG_ra; i++) {
        if (i <= ARG_d5) {
            regs[i - ARG_d0] = args[i].u_int;
        } else if (i <= ARG_a2w) {
            if (args[i].u_obj != MP_OBJ_NULL) {
                if (mp_obj_is_int(args[i].u_obj)) {
                    regs[6 + (i - ARG_a1) % 2] = mp_obj_get_int(args[i].u_obj);
                } else {
                    mp_buffer_info_t bufinfo;
                    int flags;
                    if (i <= ARG_a2) {
                        flags = MP_BUFFER_READ;
                    } else {
                        flags = MP_BUFFER_RW;
                    }
                    mp_get_buffer_raise(args[i].u_obj, &bufinfo, flags);
                    regs[6 + (i - ARG_a1) % 2] = (mp_int_t)bufinfo.buf;
                }
            }
        } else if (i == ARG_rd) {
            rd = args[i].u_int;
            if (rd < 0 || rd > 5) {
                mp_raise_ValueError(MP_ERROR_TEXT("rd out of range"));
            }
        } else if (i == ARG_ra) {
            ra = args[i].u_int;
            if (ra < 0 || ra > 2) {
                mp_raise_ValueError(MP_ERROR_TEXT("ra out of range"));
            }
        }
    }

    __asm volatile (
        "moveml %0@, %%d0-%%d5/%%a1-%%a2\n"
        "trap #15\n"
        "moveml %%d0-%%d5/%%a1-%%a2,%0@\n"
        : : "a"(regs)
        : "%%d0","%%d1","%%d2","%%d3","%%d4","%%d5","%%a1","%%a2", "memory"
    );

    if (rd + ra == 0) {
        return mp_obj_new_int(regs[0]);
    } else {
        mp_obj_t tuple[8];
        int t = 1;
        tuple[0] = mp_obj_new_int(regs[0]);
        for (i = 0; i < rd; i++) {
            tuple[t++] = mp_obj_new_int(regs[1 + i]);
        }
        for (i = 0; i < ra; i++) {
            tuple[t++] = mp_obj_new_int(regs[6 + i]);
        }
        return mp_obj_new_tuple(t, tuple);
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_KW(mp_x68k_iocs_obj, 0, mp_x68k_iocs);

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

extern const mp_obj_type_t mp_x68k_i_obj_type;

STATIC const mp_rom_map_elem_t mp_module_x68k_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_x68k) },

    { MP_ROM_QSTR(MP_QSTR_iocs), MP_ROM_PTR(&mp_x68k_iocs_obj) },
    { MP_ROM_QSTR(MP_QSTR_i), MP_ROM_PTR(&mp_x68k_i_obj_type) },

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
