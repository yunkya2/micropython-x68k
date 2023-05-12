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
#include "py/gc.h"
#include "modx68k.h"

/****************************************************************************/

typedef enum {
    INT_OPMINT,
    INT_TIMERD,
    INT_VSYNC,
    INT_CRTCRAS,
    NUM_INT_TYPES,
} int_type_t;

typedef struct _x68k_int_data_t {
    mp_obj_t callback;
    mp_obj_t arg;
    bool     softirq;
} x68k_int_data_t;
STATIC x68k_int_data_t x68k_int_data[NUM_INT_TYPES];

STATIC void int_helper(int_type_t type) {
    x68k_int_data_t *id = &x68k_int_data[type];
    mp_obj_t *cb = &id->callback;
    if (*cb != mp_const_none) {
        // If it's a soft IRQ handler then just schedule callback for later
        if (id->softirq) {
            mp_sched_schedule(*cb, id->arg);
            return;
        }

        mp_sched_lock();
        // When executing code within a handler we must lock the GC to prevent
        // any memory allocations.  We must also catch any exceptions.
        gc_lock();
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            mp_call_function_1(*cb, id->arg);
            nlr_pop();
        } else {
            // Uncaught exception; disable the callback so it doesn't run again.
            *cb = mp_const_none;
            mp_printf(MICROPY_ERROR_PRINTER, "uncaught exception in the interrupt handler %u\n", (unsigned int)type);
            mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
        }
        gc_unlock();
        mp_sched_unlock();
    }
}

/****************************************************************************/

typedef struct _x68k_intopm_t {
    mp_obj_base_t base;
} x68k_intopm_t;

__attribute__((interrupt))
STATIC void handle_intopm(void) {
    int_helper(INT_OPMINT);
}

STATIC mp_obj_t x68k_intopm_callback(size_t n_args, const mp_obj_t *args) {
    x68k_intopm_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)(self);
    mp_obj_t callback = mp_const_none;
    if (n_args > 1) {
        callback = args[1];
    }
    if (callback == mp_const_none) {
        _iocs_opmintst(0);
        x68k_int_data[INT_OPMINT].callback = mp_const_none;
    } else if (mp_obj_is_callable(callback)) {
        _iocs_opmintst(0);
        x68k_int_data[INT_OPMINT].callback = callback;
        _iocs_opmintst(handle_intopm);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be None or a callable object"));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_intopm_callback_obj, 1, 2, x68k_intopm_callback);

STATIC mp_obj_t x68k_intopm_init_helper(x68k_intopm_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_callback, ARG_arg, ARG_mode };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_arg,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    x68k_int_data[INT_OPMINT].arg = args[ARG_arg].u_obj;
    x68k_int_data[INT_OPMINT].softirq = args[ARG_mode].u_int != 0;
    mp_obj_t cb_arg[2] = { MP_OBJ_FROM_PTR(self), args[ARG_callback].u_obj };
    x68k_intopm_callback(2, cb_arg);
    return mp_const_none;
}

STATIC mp_obj_t x68k_intopm_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    x68k_intopm_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)(self);
    return x68k_intopm_init_helper(self, n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_intopm_init_obj, 1, x68k_intopm_init);

STATIC mp_obj_t x68k_intopm_deinit(mp_obj_t self_in) {
    x68k_intopm_t *self = MP_OBJ_TO_PTR(self_in);
    (void)(self);
    _iocs_opmintst(0);
    x68k_int_data[INT_OPMINT].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(x68k_intopm_deinit_obj,x68k_intopm_deinit);

STATIC mp_obj_t x68k_intopm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    x68k_intopm_t *self = mp_obj_malloc(x68k_intopm_t, type);
    if (n_args > 0 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        x68k_intopm_init_helper(self, n_args, args, &kw_args);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t x68k_intopm___exit__(size_t n_args, const mp_obj_t *args) {
    x68k_intopm_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)(self);
    _iocs_opmintst(0);
    x68k_int_data[INT_OPMINT].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_intopm___exit___obj, 4, 4, x68k_intopm___exit__);

STATIC const mp_rom_map_elem_t x68k_intopm_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&x68k_intopm_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&x68k_intopm_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_callback), MP_ROM_PTR(&x68k_intopm_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&x68k_intopm_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&x68k_intopm___exit___obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_intopm_locals_dict, x68k_intopm_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_intopm,
    MP_QSTR_IntOpm,
    MP_TYPE_FLAG_NONE,
    make_new, x68k_intopm_make_new,
    locals_dict, &x68k_intopm_locals_dict
    );

/****************************************************************************/

typedef struct _x68k_inttimerd_t {
    mp_obj_base_t base;
    mp_int_t unit;
    mp_int_t cycle;
} x68k_inttimerd_t;

__attribute__((interrupt))
STATIC void handle_inttimerd(void) {
    int_helper(INT_TIMERD);
}

STATIC mp_obj_t x68k_inttimerd_callback(size_t n_args, const mp_obj_t *args) {
    x68k_inttimerd_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_obj_t callback = mp_const_none;
    if (n_args > 1) {
        callback = args[1];
    }
    if (callback == mp_const_none) {
        _iocs_timerdst(0, 0, 0);
        x68k_int_data[INT_TIMERD].callback = mp_const_none;
    } else if (mp_obj_is_callable(callback)) {
        _iocs_timerdst(0, 0, 0);
        x68k_int_data[INT_TIMERD].callback = callback;
        _iocs_timerdst(handle_inttimerd, self->unit, self->cycle);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be None or a callable object"));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_inttimerd_callback_obj, 1, 2, x68k_inttimerd_callback);

STATIC mp_obj_t x68k_inttimerd_init_helper(x68k_inttimerd_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_callback, ARG_arg, ARG_mode, ARG_unit, ARG_cycle };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_arg,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_unit,     MP_ARG_INT, {.u_int = 1 } },
        { MP_QSTR_cycle,    MP_ARG_INT, {.u_int = 1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    x68k_int_data[INT_TIMERD].arg = args[ARG_arg].u_obj;
    x68k_int_data[INT_TIMERD].softirq = args[ARG_mode].u_int != 0;
    self->unit = args[ARG_unit].u_int;
    self->cycle = args[ARG_cycle].u_int;
    mp_obj_t cb_arg[2] = { MP_OBJ_FROM_PTR(self), args[ARG_callback].u_obj };
    x68k_inttimerd_callback(2, cb_arg);
    return mp_const_none;
}

STATIC mp_obj_t x68k_inttimerd_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    x68k_inttimerd_t *self = MP_OBJ_TO_PTR(args[0]);
    return x68k_inttimerd_init_helper(self, n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_inttimerd_init_obj, 1, x68k_inttimerd_init);

STATIC mp_obj_t x68k_inttimerd_deinit(mp_obj_t self_in) {
    x68k_inttimerd_t *self = MP_OBJ_TO_PTR(self_in);
    (void)(self);
    _iocs_timerdst(0, 0, 0);
    x68k_int_data[INT_TIMERD].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(x68k_inttimerd_deinit_obj,x68k_inttimerd_deinit);

STATIC mp_obj_t x68k_inttimerd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    x68k_inttimerd_t *self = mp_obj_malloc(x68k_inttimerd_t, type);
    if (n_args > 0 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        x68k_inttimerd_init_helper(self, n_args, args, &kw_args);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t x68k_inttimerd___exit__(size_t n_args, const mp_obj_t *args) {
    x68k_inttimerd_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)(self);
    _iocs_timerdst(0, 0, 0);
    x68k_int_data[INT_TIMERD].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_inttimerd___exit___obj, 4, 4, x68k_inttimerd___exit__);

STATIC const mp_rom_map_elem_t x68k_inttimerd_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&x68k_inttimerd_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&x68k_inttimerd_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_callback), MP_ROM_PTR(&x68k_inttimerd_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&x68k_inttimerd_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&x68k_inttimerd___exit___obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_inttimerd_locals_dict, x68k_inttimerd_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_inttimerd,
    MP_QSTR_IntTimerD,
    MP_TYPE_FLAG_NONE,
    make_new, x68k_inttimerd_make_new,
    locals_dict, &x68k_inttimerd_locals_dict
    );

/****************************************************************************/

typedef struct _x68k_intvsync_t {
    mp_obj_base_t base;
    mp_int_t disp;
    mp_int_t cycle;
} x68k_intvsync_t;

__attribute__((interrupt))
STATIC void handle_intvsync(void) {
    int_helper(INT_VSYNC);
}

STATIC mp_obj_t x68k_intvsync_callback(size_t n_args, const mp_obj_t *args) {
    x68k_intvsync_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_obj_t callback = mp_const_none;
    if (n_args > 1) {
        callback = args[1];
    }
    if (callback == mp_const_none) {
        _iocs_vdispst(0, 0, 0);
        x68k_int_data[INT_VSYNC].callback = mp_const_none;
    } else if (mp_obj_is_callable(callback)) {
        _iocs_vdispst(0, 0, 0);
        x68k_int_data[INT_VSYNC].callback = callback;
        _iocs_vdispst(handle_intvsync, self->disp, self->cycle);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be None or a callable object"));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_intvsync_callback_obj, 1, 2, x68k_intvsync_callback);

STATIC mp_obj_t x68k_intvsync_init_helper(x68k_intvsync_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_callback, ARG_arg, ARG_mode, ARG_disp, ARG_cycle };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_arg,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_disp,     MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_cycle,    MP_ARG_INT, {.u_int = 1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    x68k_int_data[INT_VSYNC].arg = args[ARG_arg].u_obj;
    x68k_int_data[INT_VSYNC].softirq = args[ARG_mode].u_int != 0;
    self->disp = args[ARG_disp].u_bool;
    self->cycle = args[ARG_cycle].u_int;
    mp_obj_t cb_arg[2] = { MP_OBJ_FROM_PTR(self), args[ARG_callback].u_obj };
    x68k_intvsync_callback(2, cb_arg);
    return mp_const_none;
}

STATIC mp_obj_t x68k_intvsync_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    x68k_intvsync_t *self = MP_OBJ_TO_PTR(args[0]);
    return x68k_intvsync_init_helper(self, n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_intvsync_init_obj, 1, x68k_intvsync_init);

STATIC mp_obj_t x68k_intvsync_deinit(mp_obj_t self_in) {
    x68k_intvsync_t *self = MP_OBJ_TO_PTR(self_in);
    (void)(self);
    _iocs_vdispst(0, 0, 0);
    x68k_int_data[INT_VSYNC].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(x68k_intvsync_deinit_obj,x68k_intvsync_deinit);

STATIC mp_obj_t x68k_intvsync_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    x68k_intvsync_t *self = mp_obj_malloc(x68k_intvsync_t, type);
    if (n_args > 0 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        x68k_intvsync_init_helper(self, n_args, args, &kw_args);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t x68k_intvsync___exit__(size_t n_args, const mp_obj_t *args) {
    x68k_intvsync_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)(self);
    _iocs_vdispst(0, 0, 0);
    x68k_int_data[INT_VSYNC].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_intvsync___exit___obj, 4, 4, x68k_intvsync___exit__);

STATIC const mp_rom_map_elem_t x68k_intvsync_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&x68k_intvsync_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&x68k_intvsync_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_callback), MP_ROM_PTR(&x68k_intvsync_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&x68k_intvsync_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&x68k_intvsync___exit___obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_intvsync_locals_dict, x68k_intvsync_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_intvsync,
    MP_QSTR_IntVSync,
    MP_TYPE_FLAG_NONE,
    make_new, x68k_intvsync_make_new,
    locals_dict, &x68k_intvsync_locals_dict
    );

/****************************************************************************/

typedef struct _x68k_intraster_t {
    mp_obj_base_t base;
    mp_int_t raster;
} x68k_intraster_t;

__attribute__((interrupt))
STATIC void handle_intraster(void) {
    int_helper(INT_CRTCRAS);
}

STATIC mp_obj_t x68k_intraster_callback(size_t n_args, const mp_obj_t *args) {
    x68k_intraster_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_obj_t callback = mp_const_none;
    if (n_args > 1) {
        callback = args[1];
    }
    if (callback == mp_const_none) {
        _iocs_crtcras(0, 0);
        x68k_int_data[INT_CRTCRAS].callback = mp_const_none;
    } else if (mp_obj_is_callable(callback)) {
        _iocs_crtcras(0, 0);
        x68k_int_data[INT_CRTCRAS].callback = callback;
        _iocs_crtcras(handle_intraster, self->raster);
    } else {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be None or a callable object"));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_intraster_callback_obj, 1, 2, x68k_intraster_callback);

STATIC mp_obj_t x68k_intraster_init_helper(x68k_intraster_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_callback, ARG_arg, ARG_mode, ARG_raster };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_arg,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_raster,   MP_ARG_INT, {.u_int = 0 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                     MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    x68k_int_data[INT_CRTCRAS].arg = args[ARG_arg].u_obj;
    x68k_int_data[INT_CRTCRAS].softirq = args[ARG_mode].u_int != 0;
    self->raster = args[ARG_raster].u_int;
    mp_obj_t cb_arg[2] = { MP_OBJ_FROM_PTR(self), args[ARG_callback].u_obj };
    x68k_intraster_callback(2, cb_arg);
    return mp_const_none;
}

STATIC mp_obj_t x68k_intraster_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    x68k_intraster_t *self = MP_OBJ_TO_PTR(args[0]);
    return x68k_intraster_init_helper(self, n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(x68k_intraster_init_obj, 1, x68k_intraster_init);

STATIC mp_obj_t x68k_intraster_deinit(mp_obj_t self_in) {
    x68k_intraster_t *self = MP_OBJ_TO_PTR(self_in);
    (void)(self);
    _iocs_crtcras(0, 0);
    x68k_int_data[INT_CRTCRAS].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(x68k_intraster_deinit_obj,x68k_intraster_deinit);

STATIC mp_obj_t x68k_intraster_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    x68k_intraster_t *self = mp_obj_malloc(x68k_intraster_t, type);
    if (n_args > 0 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        x68k_intraster_init_helper(self, n_args, args, &kw_args);
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t x68k_intraster___exit__(size_t n_args, const mp_obj_t *args) {
    x68k_intraster_t *self = MP_OBJ_TO_PTR(args[0]);
    (void)(self);
    _iocs_crtcras(0, 0);
    x68k_int_data[INT_CRTCRAS].callback = mp_const_none;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_intraster___exit___obj, 4, 4, x68k_intraster___exit__);

STATIC const mp_rom_map_elem_t x68k_intraster_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&x68k_intraster_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&x68k_intraster_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_callback), MP_ROM_PTR(&x68k_intraster_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&x68k_intraster___exit___obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_intraster_locals_dict, x68k_intraster_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_intraster,
    MP_QSTR_IntRaster,
    MP_TYPE_FLAG_NONE,
    make_new, x68k_intraster_make_new,
    locals_dict, &x68k_intraster_locals_dict
    );

/****************************************************************************/

typedef struct _x68k_intdisable_t {
    mp_obj_base_t base;
    uint16_t oldsr;
} x68k_intdisable_t;

STATIC mp_obj_t x68k_intdisable_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    x68k_intdisable_t *self = mp_obj_malloc(x68k_intdisable_t, type);
    __asm__ volatile ("movew %%sr,%0" : "=d"(self->oldsr));
    if (self->oldsr & 0x2000) {
        /* disable interrupt only when in supervisor mode */
        __asm__ volatile ("movew %0,%%sr" : : "d"(self->oldsr | 0x0700));
    }
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t x68k_intdisable___exit__(size_t n_args, const mp_obj_t *args) {
    x68k_intdisable_t *self = MP_OBJ_TO_PTR(args[0]);
    if (self->oldsr & 0x2000) {
        /* restore interrupt mask only when in supervisor mode */
        __asm__ volatile ("movew %0,%%sr" : : "d"(self->oldsr));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_intdisable___exit___obj, 4, 4, x68k_intdisable___exit__);

STATIC const mp_rom_map_elem_t x68k_intdisable_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&mp_identity_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&x68k_intdisable___exit___obj) },
};
STATIC MP_DEFINE_CONST_DICT(x68k_intdisable_locals_dict, x68k_intdisable_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_type_intdisable,
    MP_QSTR_IntDisable,
    MP_TYPE_FLAG_NONE,
    make_new, x68k_intdisable_make_new,
    locals_dict, &x68k_intdisable_locals_dict
    );

STATIC mp_obj_t x68k_intdisable(void) {
    uint16_t oldsr;
    __asm__ volatile ("movew %%sr,%0" : "=d"(oldsr));
    if (oldsr & 0x2000) {
        /* disable interrupt only when in supervisor mode */
        __asm__ volatile ("movew %0,%%sr" : : "d"(oldsr | 0x0700));
    }
    return MP_OBJ_NEW_SMALL_INT(oldsr);
}
MP_DEFINE_CONST_FUN_OBJ_0(x68k_intdisable_obj, x68k_intdisable);

STATIC mp_obj_t x68k_intenable(mp_obj_t self_in) {
    uint16_t oldsr = mp_obj_get_int(self_in);
    if (oldsr & 0x2000) {
        /* restore interrupt mask only when in supervisor mode */
        __asm__ volatile ("movew %0,%%sr" : : "d"(oldsr));
    }
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(x68k_intenable_obj, x68k_intenable);
