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

typedef struct _mp_obj_x68k_sprite_t {
    mp_obj_base_t base;
    void *buf;
    int len;
} mp_obj_x68k_sprite_t;

STATIC mp_obj_t x68k_spr_init(mp_obj_t self_in) {
    mp_int_t res = _iocs_sp_init();
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(x68k_spr_init_obj, x68k_spr_init);

STATIC mp_obj_t x68k_spr_disp(size_t n_args, const mp_obj_t *args) {
    bool onoff = true;
    mp_int_t res = 0;
    if (n_args == 2) {
        onoff = mp_obj_get_int(args[1]);
    }
    if (onoff) {
        res = _iocs_sp_on();
    } else {
        _iocs_sp_off();
    }
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_disp_obj, 1, 2, x68k_spr_disp);

STATIC mp_obj_t x68k_spr_clr(size_t n_args, const mp_obj_t *args) {
    mp_int_t from = 0, to = 255;
    mp_int_t i;
    mp_int_t res = 0;
    if (n_args == 2) {
        from = to = mp_obj_get_int(args[1]);
    } else if (n_args == 3) {
        from = mp_obj_get_int(args[1]);
        to = mp_obj_get_int(args[2]);
    }

    for (i = from; i <= to; i++) {
        res = _iocs_sp_cgclr(i);
        if (res < 0) {
            break;
        }
    }
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_clr_obj, 1, 3, x68k_spr_clr);

mp_obj_t x68k_spr_defcg(size_t n_args, const mp_obj_t *args) {
    uint8_t tmp[128];
    uint8_t *pat;
    int size = 1;
    mp_int_t res;
    if (n_args == 4) {
        size = mp_obj_get_int(args[3]);
    }
    int code = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
    pat = bufinfo.buf;
    if (size == 1) {
        for (mp_int_t i = 0; i < 16; i++) {
            for (mp_int_t j = 0; j < 4; j++) {
                tmp[i * 4 + j] = pat[i * 8 + j];
                tmp[i * 4 + j + 64] = pat[i * 8 + j + 4];
            }
        }
        pat = tmp;
    }

    res = _iocs_sp_defcg(code, (size != 0), pat);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_defcg_obj, 3, 4, x68k_spr_defcg);

STATIC mp_obj_t x68k_spr_set(size_t n_args, const mp_obj_t *args) {
    mp_int_t plane = mp_obj_get_int(args[1]);
    mp_int_t res;
    if (n_args < 6) {
        res = _iocs_sp_regst(plane, -1, 0, 0, 0, 0);
        return MP_OBJ_NEW_SMALL_INT(res);
    }

    mp_int_t x = mp_obj_get_int(args[2]);
    mp_int_t y = mp_obj_get_int(args[3]);
    mp_int_t code = mp_obj_get_int(args[4]);
    mp_int_t prio = mp_obj_get_int(args[5]);
    bool vsync = true;
    if (n_args == 7) {
        vsync = mp_obj_get_int(args[6]);
    }
    res = _iocs_sp_regst(plane, vsync ? 0 : -1, x, y, code, prio);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_set_obj, 2, 7, x68k_spr_set);

STATIC mp_obj_t x68k_spr_palet(size_t n_args, const mp_obj_t *args) {
    mp_int_t p = mp_obj_get_int(args[1]);
    mp_int_t pb = 1;
    bool vsync = true;
    if (n_args >= 4) {
        pb = mp_obj_get_int(args[3]);
    }
    if (n_args >= 5) {
        vsync = mp_obj_get_int(args[4]);
    }
    mp_int_t res = 0;
    if (mp_obj_is_int(args[2])) {
        mp_int_t c = mp_obj_get_int(args[2]);
        res = _iocs_spalet(p | (vsync ? 0 : 0x80000000), pb, c);
    } else {
        size_t i, len;
        mp_obj_t *elem;
        mp_obj_get_array(args[2], &len, &elem);
        for (i = 0; i < len; i++) {
            mp_int_t c = mp_obj_get_int(elem[i]);
            res = _iocs_spalet(p | (vsync ? 0 : 0x80000000), pb, c);
            if (res < 0) {
                break;
            }
            vsync = 0;
            p = (p + 1) % 16;
        }
    }
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_palet_obj, 3, 5, x68k_spr_palet);

STATIC mp_obj_t x68k_spr_bgdisp(size_t n_args, const mp_obj_t *args) {
    mp_int_t bg = mp_obj_get_int(args[1]);
    mp_int_t text = 0;
    mp_int_t disp = 1;
    if (n_args > 2) {
        text = mp_obj_get_int(args[2]);
    }
    if (n_args > 3) {
        disp = mp_obj_get_int(args[3]);
    }

    mp_int_t res = _iocs_bgctrlst(bg, text, disp);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_bgdisp_obj, 2, 4, x68k_spr_bgdisp);

STATIC mp_obj_t x68k_spr_bgfill(size_t n_args, const mp_obj_t *args) {
    mp_int_t text = mp_obj_get_int(args[1]);
    mp_int_t code = mp_obj_get_int(args[2]);
    mp_int_t res = _iocs_bgtextcl(text, code);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_bgfill_obj, 3, 3, x68k_spr_bgfill);

STATIC mp_obj_t x68k_spr_bgset(size_t n_args, const mp_obj_t *args) {
    mp_int_t text = mp_obj_get_int(args[1]);
    mp_int_t x = mp_obj_get_int(args[2]);
    mp_int_t y = mp_obj_get_int(args[3]);
    mp_int_t code = mp_obj_get_int(args[4]);
    mp_int_t res = _iocs_bgtextst(text, x, y, code);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_bgset_obj, 5, 5, x68k_spr_bgset);

STATIC mp_obj_t x68k_spr_bgget(size_t n_args, const mp_obj_t *args) {
    mp_int_t text = mp_obj_get_int(args[1]);
    mp_int_t x = mp_obj_get_int(args[2]);
    mp_int_t y = mp_obj_get_int(args[3]);
    mp_int_t res = _iocs_bgtextgt(text, x, y);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_bgget_obj, 4, 4, x68k_spr_bgget);

STATIC mp_obj_t x68k_spr_bgscroll(size_t n_args, const mp_obj_t *args) {
    mp_int_t bg = mp_obj_get_int(args[1]);
    mp_int_t x = mp_obj_get_int(args[2]);
    mp_int_t y = mp_obj_get_int(args[3]);
    bool vsync = true;
    if (n_args > 4) {
        vsync = mp_obj_get_int(args[4]);
    }

    mp_int_t res = _iocs_bgscrlst(bg | (vsync ? 0 : 0x80000000), x, y);
    return MP_OBJ_NEW_SMALL_INT(res);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_spr_bgscroll_obj, 4, 5, x68k_spr_bgscroll);

STATIC mp_obj_t x68k_sprite_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    mp_obj_x68k_sprite_t *o = mp_obj_malloc(mp_obj_x68k_sprite_t, type);
    o->buf = (void *)0xeb8000;
    o->len = 0x8000;
    return MP_OBJ_FROM_PTR(o);
}

STATIC mp_obj_t x68k_sprite_unary_op(mp_unary_op_t op, mp_obj_t o_in) {
    mp_obj_x68k_sprite_t *o = MP_OBJ_TO_PTR(o_in);
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

STATIC mp_obj_t x68k_sprite_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
    if (value == MP_OBJ_NULL) {
        // delete
        return MP_OBJ_NULL; // op not supported
    } else if (x68k_super_mode) {
        mp_obj_x68k_sprite_t *o = MP_OBJ_TO_PTR(self_in);
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

STATIC mp_int_t x68k_sprite_get_buffer(mp_obj_t self_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
    (void)flags;
    if (x68k_super_mode) {
        mp_obj_x68k_sprite_t *self = MP_OBJ_TO_PTR(self_in);
        bufinfo->buf = self->buf;
        bufinfo->len = self->len;
        bufinfo->typecode = 'B'; // view framebuf as bytes
    }
    return 0;
}

STATIC const mp_rom_map_elem_t x68k_sprite_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init),   MP_ROM_PTR(&x68k_spr_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_disp),   MP_ROM_PTR(&x68k_spr_disp_obj) },
    { MP_ROM_QSTR(MP_QSTR_clr),    MP_ROM_PTR(&x68k_spr_clr_obj) },
    { MP_ROM_QSTR(MP_QSTR_defcg),  MP_ROM_PTR(&x68k_spr_defcg_obj) },
    { MP_ROM_QSTR(MP_QSTR_set),    MP_ROM_PTR(&x68k_spr_set_obj) },
    { MP_ROM_QSTR(MP_QSTR_palet),  MP_ROM_PTR(&x68k_spr_palet_obj) },

    { MP_ROM_QSTR(MP_QSTR_bgdisp), MP_ROM_PTR(&x68k_spr_bgdisp_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgfill), MP_ROM_PTR(&x68k_spr_bgfill_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgput),  MP_ROM_PTR(&x68k_spr_bgset_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgset),  MP_ROM_PTR(&x68k_spr_bgset_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgget),  MP_ROM_PTR(&x68k_spr_bgget_obj) },
    { MP_ROM_QSTR(MP_QSTR_bgscroll), MP_ROM_PTR(&x68k_spr_bgscroll_obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_sprite_locals_dict, x68k_sprite_locals_dict_table);

const mp_obj_type_t x68k_type_sprite = {
    { &mp_type_type },
    .name = MP_QSTR_Sprite,
    .make_new = x68k_sprite_make_new,
    .unary_op = x68k_sprite_unary_op,
    .subscr = x68k_sprite_subscr,
    .buffer_p = { .get_buffer = x68k_sprite_get_buffer },
    .locals_dict = (mp_obj_dict_t *)&x68k_sprite_locals_dict,
};
