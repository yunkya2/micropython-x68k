/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2014 Paul Sokolovsky
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

#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "py/runtime.h"
#include "py/binary.h"
#include "py/objstr.h"
#include "py/objarray.h"
#include "modx68k.h"
#include "modx68kxarray.h"

#undef XARRAY_DEBUG

#define TYPECODE_MASK (0x7f)

STATIC mp_obj_t xarray_iterator_new(mp_obj_t array_in, mp_obj_iter_buf_t *iter_buf);

/******************************************************************************/

STATIC void xarray_print(const mp_print_t *print, mp_obj_t o_in, mp_print_kind_t kind) {
    (void)kind;
    mp_obj_x68k_xarray_t *o = MP_OBJ_TO_PTR(o_in);
    mp_printf(print, "xarray('%c'", o->typecode);

#ifdef XARRAY_DEBUG
    mp_printf(print, ", %d", xarray_sub1(o));
    if (o->dim > 1) {
        mp_printf(print, ", %d", xarray_sub2(o));
    }
    mp_printf(print, ", { len=%d, dim=%d", o->len, xarray_dim(o));
    mp_printf(print, ", items=0x%x, size=%d", (int)o->items, o->head->dim1.size);
    mp_printf(print, ", head=0x%x }", (int)o->head);
#endif

    int sub2 = (xarray_dim(o) > 1) ? xarray_sub2(o) : 0;
    if (o->len > 0) {
            mp_print_str(print, ", [");
            for (size_t i = 0; i < o->len; i++) {
                if (i > 0) {
                    mp_print_str(print, ", ");
                }
                if (sub2 > 0 && i % sub2 == 0) {
                    mp_print_str(print, "[");
                }
                mp_obj_print_helper(print, mp_binary_get_val_array(o->typecode, o->items, i), PRINT_REPR);
                if (sub2 > 0 && i % sub2 == sub2 - 1) {
                    mp_print_str(print, "]");
                }
            }
            mp_print_str(print, "]");
    }
    mp_print_str(print, ")");
}

STATIC mp_obj_x68k_xarray_t *xarray_new(char typecode, size_t dim1, size_t dim2) {
    int typecode_size = mp_binary_get_size('@', typecode, NULL);
    mp_obj_x68k_xarray_t *o = m_new_obj(mp_obj_x68k_xarray_t);
    o->base.type = &x68k_type_xarray;
    o->typecode = typecode;
    size_t bodysize;
    if (dim2 <= 0) {
        o->dim = 1;
        o->len = dim1;
        bodysize = typecode_size * o->len + sizeof(x68k_xarray_head_t);
        o->head = (x68k_xarray2_head_t *)m_new(byte, bodysize);
        o->items = (void *)o->head + sizeof(x68k_xarray_head_t);
    } else {
        o->dim = 2;
        o->len = dim1 * dim2;
        bodysize = typecode_size * o->len + sizeof(x68k_xarray2_head_t);
        o->head = (x68k_xarray2_head_t *)m_new(byte, bodysize);
        o->items = (void *)o->head + sizeof(x68k_xarray2_head_t);
        o->head->sub2 = dim2 - 1;
        o->head->dim1sz = typecode_size * dim2;
    }
    o->head->dim1.size = bodysize;
    o->head->dim1.dim = o->dim - 1;
    o->head->dim1.unitsz = typecode_size;
    o->head->dim1.sub1 = dim1 - 1;
    return o;
}

STATIC mp_obj_t xarray_construct(char typecode, size_t dim1, size_t dim2, mp_obj_t initializer) {
    mp_buffer_info_t bufinfo;
    mp_obj_x68k_xarray_t *o = NULL;

    if (dim1 > 0) {
        // create xarray with given size
        o = xarray_new(typecode, dim1, dim2);
    }

    if ((mp_obj_is_type(initializer, &mp_type_bytes)
            || (MICROPY_PY_BUILTINS_BYTEARRAY && mp_obj_is_type(initializer, &mp_type_bytearray)))
        && mp_get_buffer(initializer, &bufinfo, MP_BUFFER_READ)) {
        // construct xarray from raw bytes
        // we round-down the len to make it a multiple of sz (CPython raises error)
        size_t sz = mp_binary_get_size('@', typecode, NULL);
        size_t len = bufinfo.len / sz;
        if (!o) {
            o = xarray_new(typecode, len, 0);
        } else {
            if (o->len != len) {
                mp_raise_ValueError(MP_ERROR_TEXT("initializer size mismatch"));
            }
        }
        memcpy(o->items, bufinfo.buf, len * sz);
        return MP_OBJ_FROM_PTR(o);
    }

    // Try to get the length of the 1st initializer item
    size_t len = MP_OBJ_SMALL_INT_VALUE(mp_obj_len(initializer));
    mp_obj_t item = mp_obj_subscr(initializer, MP_OBJ_NEW_SMALL_INT(0), MP_OBJ_SENTINEL);
    mp_obj_t len_in = mp_obj_len_maybe(item);

    if (len_in == MP_OBJ_NULL) {
        // cannot be get -- initializer is 1-dimension
        if (!o) {
            o = xarray_new(typecode, len, 0);
        } else {
            if (o->len != len) {
                mp_raise_ValueError(MP_ERROR_TEXT("initializer size mismatch"));
            }
        }
        mp_obj_t iterable = mp_getiter(initializer, NULL);
        size_t i = 0;
        while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
            mp_binary_set_val_array(typecode, o->items, i++, item);
        }
    } else {
        // can be get -- initializer is 2-dimension
        size_t len2 = MP_OBJ_SMALL_INT_VALUE(len_in);
        if (!o) {
            o = xarray_new(typecode, len, len2);
        } else {
            if (xarray_sub1(o) != len || xarray_sub2(o) != len2) {
                mp_raise_ValueError(MP_ERROR_TEXT("initializer size mismatch"));
            }
        }
        mp_obj_t iterable = mp_getiter(initializer, NULL);
        size_t i = 0;
        while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION) {
            mp_obj_t item2;
            mp_obj_t iterable2 = mp_getiter(item, NULL);
            while ((item2 = mp_iternext(iterable2)) != MP_OBJ_STOP_ITERATION) {
                mp_binary_set_val_array(typecode, o->items, i++, item2);
            }
        }
    }

    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t xarray_make_new_main(char typecode, size_t n_args, const mp_obj_t *args) {
    switch (n_args) {
        case 0:     // xarray(type)
            return MP_OBJ_FROM_PTR(xarray_new(typecode, 1, 0));
        case 1:     // xarray(type, dim1) or xarray(type, initializer)
            if (mp_obj_is_int(args[0])) {
                size_t dim1 = mp_obj_get_int(args[0]);
                return MP_OBJ_FROM_PTR(xarray_new(typecode, dim1, 0));
            } else {
                return xarray_construct(typecode, 0, 0, args[0]);
            }
            break;
        case 2:     // xarray(type, dim1, dim2) or xarray(type, dim1, initializer)
            if (mp_obj_is_int(args[1])) {
                size_t dim1 = mp_obj_get_int(args[0]);
                size_t dim2 = mp_obj_get_int(args[1]);
                return MP_OBJ_FROM_PTR(xarray_new(typecode, dim1, dim2));
            } else {
                size_t dim1 = mp_obj_get_int(args[0]);
                return xarray_construct(typecode, dim1, 0, args[1]);
            }
            break;
        case 3:     // xarray(type, dim1, dim2, initializer)
            size_t dim1 = mp_obj_get_int(args[0]);
            size_t dim2 = mp_obj_get_int(args[1]);
            return xarray_construct(typecode, dim1, dim2, args[2]);
            break;
    }
    return mp_const_none;
}

STATIC mp_obj_t xarray_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    (void)type_in;
    mp_arg_check_num(n_args, n_kw, 1, 4, false);

    // get typecode
    const char *typecode = mp_obj_str_get_str(args[0]);
    return xarray_make_new_main(*typecode, n_args - 1, args + 1);
}

STATIC mp_obj_t x68k_xarray_char(size_t n_args, const mp_obj_t *args) {
    return xarray_make_new_main('B', n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_xarray_char_obj, 0, 3, x68k_xarray_char);

STATIC mp_obj_t x68k_xarray_int(size_t n_args, const mp_obj_t *args) {
    return xarray_make_new_main('l', n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_xarray_int_obj, 0, 3, x68k_xarray_int);

STATIC mp_obj_t x68k_xarray_float(size_t n_args, const mp_obj_t *args) {
    return xarray_make_new_main('d', n_args, args);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_xarray_float_obj, 0, 3, x68k_xarray_float);

STATIC mp_obj_t xarray_unary_op(mp_unary_op_t op, mp_obj_t o_in) {
    mp_obj_x68k_xarray_t *o = MP_OBJ_TO_PTR(o_in);
    switch (op) {
        case MP_UNARY_OP_LEN:
            return MP_OBJ_NEW_SMALL_INT(o->len);
        default:
            return MP_OBJ_NULL;      // op not supported
    }
}

STATIC mp_obj_t xarray_get_val(mp_obj_x68k_xarray_t *o, size_t index) {
    if (xarray_dim(o) > 1) {
        // create new array object pointing to single column
        index *= xarray_sub2(o);
        if (index >= o->len) {
            return MP_OBJ_SENTINEL;
        }
        mp_obj_array_t *col = m_new_obj(mp_obj_array_t);
        col->base.type = &mp_type_array;
        col->typecode = o->typecode;
        col->free = 0;
        col->len = xarray_sub2(o);
        col->items = o->items + index * o->head->dim1.unitsz;
        return MP_OBJ_FROM_PTR(col);
    } else {
        return mp_binary_get_val_array(o->typecode & TYPECODE_MASK, o->items, index);
    }
}

STATIC mp_obj_t xarray_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    if (value == MP_OBJ_NULL) {
        // delete item
        return MP_OBJ_NULL; // op not supported
    } else {
        mp_obj_x68k_xarray_t *o = MP_OBJ_TO_PTR(self_in);
        {
            size_t index = mp_get_index(o->base.type, o->len, index_in, false);
            if (value == MP_OBJ_SENTINEL) {
                // load
                mp_obj_t res = xarray_get_val(o, index);
                if (res == MP_OBJ_SENTINEL) {
                    mp_raise_msg(&mp_type_IndexError, MP_ERROR_TEXT("xarray index out of range"));
                }
                return res;
            } else {
                // store
                if (xarray_dim(o) > 1) {
                    return MP_OBJ_NULL; // op not supported
                }
                mp_binary_set_val_array(o->typecode & TYPECODE_MASK, o->items, index, value);
                return mp_const_none;
            }
        }
    }
}

STATIC mp_int_t xarray_get_buffer(mp_obj_t o_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
    mp_obj_x68k_xarray_t *o = MP_OBJ_TO_PTR(o_in);
    size_t sz = mp_binary_get_size('@', o->typecode & TYPECODE_MASK, NULL);
    bufinfo->buf = o->items;
    bufinfo->len = o->len * sz;
    bufinfo->typecode = o->typecode & TYPECODE_MASK;
    (void)flags;
    return 0;
}

STATIC mp_obj_t xarray_shape(mp_obj_t o_in) {
    mp_obj_x68k_xarray_t *o = MP_OBJ_TO_PTR(o_in);
    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(o->dim, NULL));
    t->items[0] = MP_OBJ_NEW_SMALL_INT(xarray_sub1(o));
    if (xarray_dim(o) > 1) {
        t->items[1] = MP_OBJ_NEW_SMALL_INT(xarray_sub2(o));
    }
    return MP_OBJ_FROM_PTR(t);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(xarray_shape_obj, xarray_shape);

STATIC mp_obj_t xarray_list(size_t n_args, const mp_obj_t *args) {
    mp_obj_x68k_xarray_t *o = MP_OBJ_TO_PTR(args[0]);
    mp_obj_list_t *l = m_new_obj(mp_obj_list_t);
    if (xarray_dim(o) == 1 || (n_args > 1 && mp_obj_is_true(args[1]))) {
        // create 1-dimension list
        mp_obj_list_init(l, o->len);
        for (size_t i = 0; i < o->len; i++) {
            l->items[i] = mp_binary_get_val_array(o->typecode & TYPECODE_MASK, o->items, i);
        }
    } else {
        // create 2-dimension list
        mp_obj_list_init(l, xarray_sub1(o));
        for (size_t i = 0; i < xarray_sub1(o); i++) {
            mp_obj_list_t *m = m_new_obj(mp_obj_list_t);
            mp_obj_list_init(m, xarray_sub2(o));
            for (size_t j = 0; j < xarray_sub2(o); j++) {
                m->items[j] = mp_binary_get_val_array(o->typecode & TYPECODE_MASK, o->items, i * xarray_sub2(o) + j);
            }
            l->items[i] = m;
        }
    }
    return MP_OBJ_FROM_PTR(l);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(xarray_list_obj, 1, 2, xarray_list);

STATIC const mp_rom_map_elem_t xarray_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_shape), MP_ROM_PTR(&xarray_shape_obj) },
    { MP_ROM_QSTR(MP_QSTR_list), MP_ROM_PTR(&xarray_list_obj) },
};
STATIC MP_DEFINE_CONST_DICT(xarray_locals_dict, xarray_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_xarray,
    MP_QSTR_xarray,
    MP_TYPE_FLAG_ITER_IS_GETITER,
    make_new, xarray_make_new,
    print, xarray_print,
    iter, xarray_iterator_new,
    unary_op, xarray_unary_op,
    subscr, xarray_subscr,
    buffer, xarray_get_buffer,
    locals_dict, &xarray_locals_dict
    );

/******************************************************************************/
// xarray iterator

typedef struct _mp_obj_xarray_it_t {
    mp_obj_base_t base;
    mp_obj_x68k_xarray_t *xarray;
    size_t cur;
} mp_obj_xarray_it_t;

STATIC mp_obj_t xarray_it_iternext(mp_obj_t self_in) {
    mp_obj_xarray_it_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->cur < self->xarray->len) {
        mp_obj_t res = xarray_get_val(self->xarray, self->cur++);
        if (res == MP_OBJ_SENTINEL) {
            return MP_OBJ_STOP_ITERATION;
        } else {
            return res;
        }
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

STATIC MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_xarray_it,
    MP_QSTR_iterator,
    MP_TYPE_FLAG_ITER_IS_ITERNEXT,
    iter, xarray_it_iternext
    );

STATIC mp_obj_t xarray_iterator_new(mp_obj_t xarray_in, mp_obj_iter_buf_t *iter_buf) {
    assert(sizeof(mp_obj_xarray_it_t) <= sizeof(mp_obj_iter_buf_t));
    mp_obj_x68k_xarray_t *xarray = MP_OBJ_TO_PTR(xarray_in);
    mp_obj_xarray_it_t *o = (mp_obj_xarray_it_t *)iter_buf;
    o->base.type = &x68k_type_xarray_it;
    o->xarray = xarray;
    o->cur = 0;
    return MP_OBJ_FROM_PTR(o);
}
