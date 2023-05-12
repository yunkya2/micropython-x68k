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

STATIC mp_obj_t x68k_vpage(const mp_obj_t arg_in) {
    mp_int_t page = mp_obj_get_int(arg_in);
    int res = _iocs_vpage(page);
    return MP_OBJ_NEW_SMALL_INT(res);
}
MP_DEFINE_CONST_FUN_OBJ_1(x68k_vpage_obj, x68k_vpage);

/****************************************************************************/

typedef struct _mp_obj_x68k_gvram_t {
    mp_obj_base_t base;
    void *buf;
    int len;
    int page;
} mp_obj_x68k_gvram_t;

STATIC int current_page = -1;

STATIC void apage(mp_obj_x68k_gvram_t *self) {
    if (self->page != current_page) {
        _iocs_apage(self->page);
        current_page = self->page;
    }
}

STATIC mp_obj_t x68k_gvram_palet(mp_obj_t self_in, mp_obj_t arg1, mp_obj_t arg2) {
    mp_int_t pal = mp_obj_get_int(arg1);
    mp_int_t col = mp_obj_get_int(arg2);
    mp_int_t res = _iocs_gpalet(pal, col);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(x68k_gvram_palet_obj, x68k_gvram_palet);

STATIC mp_obj_t x68k_gvram_home(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_all };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_all, MP_ARG_INT, {.u_int = 0} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    int page = 0;
    if (!args[ARG_all].u_int) {
        page = 1 << self->page;
    }

    int res = _iocs_home(page, args[ARG_x].u_int, args[ARG_y].u_int);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_home_obj, 1, x68k_gvram_home);

STATIC mp_obj_t x68k_gvram_window(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1 };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    int res = _iocs_window(args[ARG_x0].u_int, args[ARG_y0].u_int,
                           args[ARG_x1].u_int, args[ARG_y1].u_int);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_window_obj, 2, x68k_gvram_window);

STATIC mp_obj_t x68k_gvram_wipe(mp_obj_t self_in) {
    mp_obj_x68k_gvram_t *self = MP_OBJ_TO_PTR(self_in);
    apage(self);
    int res = _iocs_wipe();
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(x68k_gvram_wipe_obj, x68k_gvram_wipe);

STATIC mp_obj_t x68k_gvram_pset(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_c };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_c, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_psetptr p = { 
        args[ARG_x].u_int, args[ARG_y].u_int,
        args[ARG_c].u_int
    };
    int res = _iocs_pset(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_pset_obj, 1, x68k_gvram_pset);

STATIC mp_obj_t x68k_gvram_point(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_pointptr p = { 
        args[ARG_x].u_int, args[ARG_y].u_int,
        0
    };
    int res = _iocs_point(&p);
    if (res < 0) {
        return MP_OBJ_NEW_SMALL_INT(res);
    } else {
        return MP_OBJ_NEW_SMALL_INT(p.color);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_point_obj, 1, x68k_gvram_point);

STATIC mp_obj_t x68k_gvram_line(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1, ARG_c, ARG_style };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_c,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_style, MP_ARG_INT, {.u_int = 0xffff} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_lineptr p = { 
        args[ARG_x0].u_int, args[ARG_y0].u_int,
        args[ARG_x1].u_int, args[ARG_y1].u_int,
        args[ARG_c].u_int, args[ARG_style].u_int
    };
    int res = _iocs_line(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_line_obj, 1, x68k_gvram_line);

STATIC mp_obj_t x68k_gvram_box(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1, ARG_c, ARG_style };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_c,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_style, MP_ARG_INT, {.u_int = 0xffff} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_boxptr p = { 
        args[ARG_x0].u_int, args[ARG_y0].u_int,
        args[ARG_x1].u_int, args[ARG_y1].u_int,
        args[ARG_c].u_int, args[ARG_style].u_int
    };
    int res = _iocs_box(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_box_obj, 1, x68k_gvram_box);

STATIC mp_obj_t x68k_gvram_fill(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1, ARG_c };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_c,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_fillptr p = { 
        args[ARG_x0].u_int, args[ARG_y0].u_int,
        args[ARG_x1].u_int, args[ARG_y1].u_int,
        args[ARG_c].u_int,
    };
    int res = _iocs_fill(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_fill_obj, 1, x68k_gvram_fill);

STATIC mp_obj_t x68k_gvram_circle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_r, ARG_c, ARG_start, ARG_end, ARG_ratio };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_r,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_c,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_start, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_end,   MP_ARG_INT, {.u_int = 360} },
        { MP_QSTR_ratio, MP_ARG_INT, {.u_int = 256} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    struct iocs_circleptr p = { 
        args[ARG_x].u_int, args[ARG_y].u_int,
        args[ARG_r].u_int, args[ARG_c].u_int,
        args[ARG_start].u_int, args[ARG_end].u_int,
        args[ARG_ratio].u_int
    };
    int res = _iocs_circle(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_circle_obj, 1, x68k_gvram_circle);

STATIC mp_obj_t x68k_gvram_paint(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_c, ARG_buf };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_c,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_buf, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    char work[1024];
    void *buf_start, *buf_end;    

    if (args[ARG_buf].u_obj == MP_OBJ_NULL) {
        buf_start = work;
        buf_end = work + sizeof(work);
    } else {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_RW);
        buf_start = bufinfo.buf;
        buf_end = bufinfo.buf + bufinfo.len;
    }

    struct iocs_paintptr p = { 
        args[ARG_x].u_int, args[ARG_y].u_int,
        args[ARG_c].u_int,
        buf_start, buf_end
    };
    int res = _iocs_paint(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_paint_obj, 1, x68k_gvram_paint);

STATIC mp_obj_t x68k_gvram_symbol(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_y, ARG_str, ARG_xmag, ARG_ymag, ARG_c, ARG_ftype, ARG_angle };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_str,   MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_xmag,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_ymag,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_c,     MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_ftype, MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_angle, MP_ARG_INT, {.u_int = 0} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    const char *str = mp_obj_str_get_str(args[ARG_str].u_obj);

    struct iocs_symbolptr p = { 
        args[ARG_x].u_int, args[ARG_y].u_int, (const unsigned char *)str,
        args[ARG_xmag].u_int, args[ARG_ymag].u_int,
        args[ARG_c].u_int, args[ARG_ftype].u_int, args[ARG_angle].u_int
    };
    int res = _iocs_symbol(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_symbol_obj, 1, x68k_gvram_symbol);

STATIC mp_obj_t x68k_gvram_get(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1, ARG_buf };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_buf, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    void *buf_start, *buf_end;    
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_WRITE);
    buf_start = bufinfo.buf;
    buf_end = bufinfo.buf + bufinfo.len;

    struct iocs_getptr p = { 
        args[ARG_x0].u_int, args[ARG_y0].u_int,
        args[ARG_x1].u_int, args[ARG_y1].u_int,
        buf_start, buf_end
    };
    int res = _iocs_getgrm(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_get_obj, 1, x68k_gvram_get);

STATIC mp_obj_t x68k_gvram_put(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x0, ARG_y0, ARG_x1, ARG_y1, ARG_buf };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x0,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y0,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_x1,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_y1,  MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_buf, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };

    mp_obj_x68k_gvram_t *self = pos_args[0];
    apage(self);

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    void *buf_start, *buf_end;    
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_READ);
    buf_start = bufinfo.buf;
    buf_end = bufinfo.buf + bufinfo.len;

    struct iocs_putptr p = { 
        args[ARG_x0].u_int, args[ARG_y0].u_int,
        args[ARG_x1].u_int, args[ARG_y1].u_int,
        buf_start, buf_end
    };
    int res = _iocs_putgrm(&p);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_gvram_put_obj, 1, x68k_gvram_put);

STATIC mp_obj_t x68k_gvram_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);

    mp_obj_x68k_gvram_t *o = mp_obj_malloc(mp_obj_x68k_gvram_t, type);
    o->page = 0;
    if (n_args > 0) {
        o->page = mp_obj_get_int(args[0]);
    }
    if (o->page == 0) {
        o->buf = (void *)0xc00000;
        o->len = 0x200000;
    } else {
        o->buf = (void *)(0xc00000 + 0x80000 * o->page);
        o->len = 0x80000;
    }
    current_page = -1;
    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t x68k_gvram_unary_op(mp_unary_op_t op, mp_obj_t o_in) {
    mp_obj_x68k_gvram_t *o = MP_OBJ_TO_PTR(o_in);
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

STATIC mp_obj_t x68k_gvram_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    if (value == MP_OBJ_NULL) {
        // delete
        return MP_OBJ_NULL; // op not supported
    } else if (x68k_super_mode) {
        mp_obj_x68k_gvram_t *o = MP_OBJ_TO_PTR(self_in);
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

STATIC mp_int_t x68k_gvram_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
    (void)flags;
    if (x68k_super_mode) {
        mp_obj_x68k_gvram_t *self = MP_OBJ_TO_PTR(self_in);
        bufinfo->buf = self->buf;
        bufinfo->len = self->len;
        bufinfo->typecode = 'B'; // view framebuf as bytes
    }
    return 0;
}

STATIC const mp_rom_map_elem_t x68k_gvram_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_palet),  MP_ROM_PTR(&x68k_gvram_palet_obj) },
    { MP_ROM_QSTR(MP_QSTR_home),   MP_ROM_PTR(&x68k_gvram_home_obj) },
    { MP_ROM_QSTR(MP_QSTR_window), MP_ROM_PTR(&x68k_gvram_window_obj) },
    { MP_ROM_QSTR(MP_QSTR_wipe),   MP_ROM_PTR(&x68k_gvram_wipe_obj) },
    { MP_ROM_QSTR(MP_QSTR_pset),   MP_ROM_PTR(&x68k_gvram_pset_obj) },
    { MP_ROM_QSTR(MP_QSTR_point),  MP_ROM_PTR(&x68k_gvram_point_obj) },
    { MP_ROM_QSTR(MP_QSTR_line),   MP_ROM_PTR(&x68k_gvram_line_obj) },
    { MP_ROM_QSTR(MP_QSTR_box),    MP_ROM_PTR(&x68k_gvram_box_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill),   MP_ROM_PTR(&x68k_gvram_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&x68k_gvram_circle_obj) },
    { MP_ROM_QSTR(MP_QSTR_paint),  MP_ROM_PTR(&x68k_gvram_paint_obj) },
    { MP_ROM_QSTR(MP_QSTR_symbol), MP_ROM_PTR(&x68k_gvram_symbol_obj) },
    { MP_ROM_QSTR(MP_QSTR_get),    MP_ROM_PTR(&x68k_gvram_get_obj) },
    { MP_ROM_QSTR(MP_QSTR_put),    MP_ROM_PTR(&x68k_gvram_put_obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_gvram_locals_dict, x68k_gvram_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_gvram,
    MP_QSTR_GVRam,
    MP_TYPE_FLAG_NONE,
    make_new, x68k_gvram_make_new,
    unary_op, x68k_gvram_unary_op,
    subscr, x68k_gvram_subscr,
    buffer, x68k_gvram_get_buffer,
    locals_dict, &x68k_gvram_locals_dict
    );
