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
#ifndef MICROPY_INCLUDED_MODX68K_H
#define MICROPY_INCLUDED_MODX68K_H

#include "py/obj.h"

extern bool x68k_super_mode;

MP_DECLARE_CONST_FUN_OBJ_1(x68k_vpage_obj);
extern const mp_obj_type_t x68k_type_gvram;

extern const mp_obj_type_t x68k_type_tvram;

extern const mp_obj_type_t x68k_type_sprite;

MP_DECLARE_CONST_FUN_OBJ_KW(x68k_iocs_obj);
extern const mp_obj_type_t x68k_i_obj_type;

MP_DECLARE_CONST_FUN_OBJ_KW(x68k_dos_obj);
extern const mp_obj_type_t x68k_d_obj_type;

extern const mp_obj_type_t x68k_type_intopm;
extern const mp_obj_type_t x68k_type_inttimerd;
extern const mp_obj_type_t x68k_type_intvsync;
extern const mp_obj_type_t x68k_type_intraster;
extern const mp_obj_type_t x68k_type_intdisable;
MP_DECLARE_CONST_FUN_OBJ_0(x68k_intdisable_obj);
MP_DECLARE_CONST_FUN_OBJ_1(x68k_intenable_obj);

MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_loadfnc_obj);
extern const mp_obj_type_t x68k_type_xarray;
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_xarray_char_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_xarray_int_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_xarray_float_obj);

#endif // MICROPY_INCLUDED_MODX68K_H
