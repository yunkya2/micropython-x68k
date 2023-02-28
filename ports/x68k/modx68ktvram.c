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
#include "py/binary.h"
#include "modx68k.h"

typedef struct _mp_obj_x68k_tvram_t {
    mp_obj_base_t base;
    void *buf;
    int len;
} mp_obj_x68k_tvram_t;

STATIC mp_obj_t x68k_tvram_palet(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2) {
    mp_int_t pal = mp_obj_get_int(arg1);
    mp_int_t col = mp_obj_get_int(arg2);
    _iocs_tpalet(pal, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(x68k_tvram_palet_obj, x68k_tvram_palet);

STATIC mp_obj_t x68k_tvram_palet2(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2) {
    mp_int_t pal = mp_obj_get_int(arg1);
    mp_int_t col = mp_obj_get_int(arg2);
    _iocs_tpalet2(pal, col);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(x68k_tvram_palet2_obj, x68k_tvram_palet2);

STATIC mp_obj_t x68k_tvram_xline(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_plane, ARG_x, ARG_y, ARG_len, ARG_style };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_plane,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_len,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_style,  MP_ARG_INT, {.u_int = 0xffff} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_xlineptr p = { 
        args[ARG_plane].u_int,
        args[ARG_x].u_int, args[ARG_y].u_int, 
        args[ARG_len].u_int, args[ARG_style].u_int
    };
    _iocs_txxline(&p);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_xline_obj, 1, x68k_tvram_xline);

STATIC mp_obj_t x68k_tvram_yline(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_plane, ARG_x, ARG_y, ARG_len, ARG_style };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_plane,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_len,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_style,  MP_ARG_INT, {.u_int = 0xffff} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_ylineptr p = { 
        args[ARG_plane].u_int,
        args[ARG_x].u_int, args[ARG_y].u_int, 
        args[ARG_len].u_int, args[ARG_style].u_int
    };
    _iocs_txyline(&p);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_yline_obj, 1, x68k_tvram_yline);

STATIC mp_obj_t x68k_tvram_line(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_plane, ARG_x, ARG_y, ARG_w, ARG_h, ARG_style };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_plane,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_w,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_h,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_style,  MP_ARG_INT, {.u_int = 0xffff} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_tlineptr p = { 
        args[ARG_plane].u_int,
        args[ARG_x].u_int, args[ARG_y].u_int, 
        args[ARG_w].u_int, args[ARG_h].u_int, 
        args[ARG_style].u_int
    };
    _iocs_txline(&p);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_line_obj, 1, x68k_tvram_line);

STATIC mp_obj_t x68k_tvram_box(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_plane, ARG_x, ARG_y, ARG_w, ARG_h, ARG_style };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_plane,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_w,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_h,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_style,  MP_ARG_INT, {.u_int = 0xffff} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_tboxptr p = { 
        args[ARG_plane].u_int,
        args[ARG_x].u_int, args[ARG_y].u_int, 
        args[ARG_w].u_int, args[ARG_h].u_int, 
        args[ARG_style].u_int
    };
    _iocs_txbox(&p);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_box_obj, 1, x68k_tvram_box);

STATIC mp_obj_t x68k_tvram_fill(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_plane, ARG_x, ARG_y, ARG_w, ARG_h, ARG_style };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_plane,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_w,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_h,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_style,  MP_ARG_INT, {.u_int = 0xffff} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_txfillptr p = { 
        args[ARG_plane].u_int,
        args[ARG_x].u_int, args[ARG_y].u_int, 
        args[ARG_w].u_int, args[ARG_h].u_int, 
        args[ARG_style].u_int
    };
    _iocs_txfill(&p);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_fill_obj, 1, x68k_tvram_fill);

STATIC mp_obj_t x68k_tvram_rev(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_plane, ARG_x, ARG_y, ARG_w, ARG_h };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_plane,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_w,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_h,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_trevptr p = { 
        args[ARG_plane].u_int,
        args[ARG_x].u_int, args[ARG_y].u_int, 
        args[ARG_w].u_int, args[ARG_h].u_int
    };
    _iocs_txrev(&p);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_rev_obj, 1, x68k_tvram_rev);

STATIC mp_obj_t x68k_tvram_rascpy(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_src, ARG_dst, ARG_n, ARG_dir, ARG_plane };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_src,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dst,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_n,      MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_dir,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_plane,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    _iocs_txrascpy(args[ARG_src].u_int << 8 | args[ARG_dst].u_int,
                   args[ARG_n].u_int,
                   (args[ARG_dir].u_int >=0 ? 0 : 0xff) << 8 | args[ARG_plane].u_int);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_rascpy_obj, 1, x68k_tvram_rascpy);

STATIC mp_obj_t x68k_tvram_color(mp_obj_t self_in, const mp_obj_t arg_in) {
    mp_int_t plane = mp_obj_get_int(arg_in);
    _iocs_tcolor(plane);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(x68k_tvram_color_obj, x68k_tvram_color);

STATIC mp_obj_t x68k_tvram_get(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_buf };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_buf, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_WRITE);

    _iocs_textget(args[ARG_x].u_int, args[ARG_y].u_int, bufinfo.buf);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_get_obj, 1, x68k_tvram_get);

STATIC mp_obj_t x68k_tvram_put(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_buf };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_buf, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_READ);

    _iocs_textput(args[ARG_x].u_int, args[ARG_y].u_int, bufinfo.buf);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_put_obj, 1, x68k_tvram_put);

STATIC mp_obj_t x68k_tvram_clipput(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_buf, ARG_clip };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_buf,  MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_clip, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_READ);

    size_t len;
    mp_obj_t *elem;
    mp_obj_get_array(args[ARG_clip].u_obj, &len, &elem);
    if (len != 4) {
        mp_raise_TypeError(MP_ERROR_TEXT("clipput needs a tuple of length 4"));
    }

    struct iocs_clipxy clip = {
        mp_obj_get_int(elem[0]),
        mp_obj_get_int(elem[1]),
        mp_obj_get_int(elem[2]),
        mp_obj_get_int(elem[3])
    };
    _iocs_clipput(args[ARG_x].u_int, args[ARG_y].u_int, bufinfo.buf, &clip);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_tvram_clipput_obj, 1, x68k_tvram_clipput);

STATIC mp_obj_t x68k_tvram_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    mp_obj_x68k_tvram_t *o = mp_obj_malloc(mp_obj_x68k_tvram_t, type);
    o->buf = (void *)0xe00000;
    o->len = 0x80000;
    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t x68k_tvram_unary_op(mp_unary_op_t op, mp_obj_t o_in) {
    mp_obj_x68k_tvram_t *o = MP_OBJ_TO_PTR(o_in);
    switch (op) {
        case MP_UNARY_OP_LEN:
            if (x68k_super_mode) {
                return MP_OBJ_NEW_SMALL_INT(o->len);
            }
            /* fall through */
        default:
            return MP_OBJ_NULL;      // op not supported
    }
}

STATIC mp_obj_t x68k_tvram_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    if (value == MP_OBJ_NULL) {
        // delete
        return MP_OBJ_NULL; // op not supported
    } else if (x68k_super_mode) {
        mp_obj_x68k_tvram_t *o = MP_OBJ_TO_PTR(self_in);
        {
            size_t index = mp_get_index(o->base.type, o->len, index_in, false);
            if (value == MP_OBJ_SENTINEL) {
                // load
                return mp_binary_get_val_array('B', o->buf, index);
            } else {
                // store
                mp_binary_set_val_array('B', o->buf, index, value);
                return mp_const_none;
            }
        }
    }
    return MP_OBJ_NULL;
}

STATIC mp_int_t x68k_tvram_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
    (void)flags;
    if (x68k_super_mode) {
        mp_obj_x68k_tvram_t *self = MP_OBJ_TO_PTR(self_in);
        bufinfo->buf = self->buf;
        bufinfo->len = self->len;
        bufinfo->typecode = 'B'; // view framebuf as bytes
    }
    return 0;
}

STATIC const mp_rom_map_elem_t x68k_tvram_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_palet),   MP_ROM_PTR(&x68k_tvram_palet_obj) },
    { MP_ROM_QSTR(MP_QSTR_palet2),  MP_ROM_PTR(&x68k_tvram_palet2_obj) },
    { MP_ROM_QSTR(MP_QSTR_xline),   MP_ROM_PTR(&x68k_tvram_xline_obj) },
    { MP_ROM_QSTR(MP_QSTR_yline),   MP_ROM_PTR(&x68k_tvram_yline_obj) },
    { MP_ROM_QSTR(MP_QSTR_line),    MP_ROM_PTR(&x68k_tvram_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_box),     MP_ROM_PTR(&x68k_tvram_box_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill),    MP_ROM_PTR(&x68k_tvram_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_rev),     MP_ROM_PTR(&x68k_tvram_rev_obj) },
    { MP_ROM_QSTR(MP_QSTR_rascpy),  MP_ROM_PTR(&x68k_tvram_rascpy_obj) },
    { MP_ROM_QSTR(MP_QSTR_color),   MP_ROM_PTR(&x68k_tvram_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_get),     MP_ROM_PTR(&x68k_tvram_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_put),     MP_ROM_PTR(&x68k_tvram_put_obj) },
    { MP_ROM_QSTR(MP_QSTR_clipput), MP_ROM_PTR(&x68k_tvram_clipput_obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_tvram_locals_dict, x68k_tvram_locals_dict_table);

const mp_obj_type_t x68k_type_tvram = {
    { &mp_type_type },
    .name = MP_QSTR_TVRam,
    .make_new = x68k_tvram_make_new,
    .unary_op = x68k_tvram_unary_op,
    .subscr = x68k_tvram_subscr,
    .buffer_p = { .get_buffer = x68k_tvram_get_buffer },
    .locals_dict = (mp_obj_dict_t *)&x68k_tvram_locals_dict,
};
