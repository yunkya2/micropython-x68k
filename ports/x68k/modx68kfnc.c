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
#include <string.h>
#include <errno.h>
#include <x68k/dos.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/obj.h"
#include "modx68k.h"
#include "modx68kxarray.h"

#undef XFNC_DEBUG

/* X-BASIC FNC file information table format */
typedef struct _x68k_fnc_info_t {
    union {
        void (*entry_boot)(void);
        struct _x68k_fnc_info_t *next;
    };
    void (*entry_run)(void);
    void (*entry_end)(void);
    void (*entry_exit)(void);
    void (*entry_break)(void);
    void (*entry_ctrld)(void);
    void (*entry_dummy0)(void);
    void (*entry_dummy1)(void);
    const char *tokentbl;
    const uint16_t **parmtbl;
    const void **entrytbl;
} x68k_fnc_info_t;

/* X-BASIC function argument format */
typedef struct _x68k_fnc_arg_t {
    uint16_t type;
    union {
        double f;
        uint64_t v;
        struct { uint32_t h; uint32_t l; } i;
    } fac;
} x68k_fnc_arg_t;

/* Function entry structure */
typedef struct _mp_obj_fun_x68kfnc_t {
    struct {
        /* Trampoline instructions to jump to common entry */
        uint16_t op0;
        uint32_t op0addr;
        uint16_t op1;
        uint32_t op1addr;
        uint16_t op2;
    } instr __attribute__((packed));

    mp_obj_fun_builtin_var_t fncobj;
    const void *fncentry;       // Function entry address
    const uint16_t *parmtbl;    // Parameter ID table address
    int n_fncargs;              // # of function arguments
    int n_optargs;              // # of optional arguments
    size_t fncarg_sz;           // Size of the function arguments in stack
} mp_obj_fun_x68kfnc_t;

MP_DEFINE_CONST_FUN_OBJ_VAR(x68k_fnc_obj, 0, 0);
static uint32_t x68kfnc_t_addr;

static x68k_fnc_info_t *xfnc_list = NULL;

/****************************************************************************/

STATIC mp_obj_t x68k_callfncentry(size_t n_args, const mp_obj_t *args) {
    mp_obj_fun_x68kfnc_t *self = (mp_obj_fun_x68kfnc_t *)(x68kfnc_t_addr - 14);
    uint16_t parmid;        // Parameter ID
    uint16_t *fncarg;       // Function arguments top address
    x68k_fnc_arg_t *farg;   // Function argument
    int n_opts;
    int i;
    int j;

    if (n_args > self->n_fncargs) {
        mp_raise_TypeError(MP_ERROR_TEXT("too many function arguments"));
    }
    if (n_args < self->n_fncargs - self->n_optargs) {
        mp_raise_TypeError(MP_ERROR_TEXT("missing function arguments"));
    }
    n_opts = n_args - (self->n_fncargs - self->n_optargs);

    fncarg = alloca(self->fncarg_sz);
    *fncarg = self->n_fncargs;
    farg = (x68k_fnc_arg_t *)&fncarg[1];

    j = 0;
    for (i = 0; i < self->n_fncargs; i++, farg++) {
        parmid = self->parmtbl[i];
        farg->fac.v = 0;

        if (parmid & 0x80) {        // option argument
            if (n_opts > 0) {
                n_opts--;
            } else {
                farg->type = -1;
                farg->fac.v = -1;
                continue;
            }
        }

        if (parmid & 0x10) {        // structured argument (array/pointer)
            extern const mp_obj_type_t x68k_type_xarray;
            if (!mp_obj_is_type(args[j], &x68k_type_xarray)) {
                mp_raise_TypeError(MP_ERROR_TEXT("xarray argument required"));
            }
            mp_obj_x68k_xarray_t *o = MP_OBJ_TO_PTR(args[j++]);
            if (((parmid & 0x40) && o->dim != 2) || (!(parmid & 0x40) && o->dim != 1)) {
                mp_raise_TypeError(MP_ERROR_TEXT("xarray dimension mismatch"));
            }
            if ((parmid & 0x60) == 0) {     // pointer
                farg->fac.i.l = (uint32_t)o->items;
            } else {                        // array
                farg->fac.i.l = (uint32_t)o->head;
            }
            switch (o->head->dim1.unitsz) {
                case 1:
                    if (parmid & 0x04) {
                        farg->type = 2;     // char
                        continue;
                    }
                    break;
                case 4:
                    if (parmid & 0x02) {
                        farg->type = 1;     // int
                        continue;
                    }
                    break;
                case 8:
                    if (parmid & 0x01) {
                        farg->type = 0;     // float
                        continue;
                    }
                    break;
            }
            mp_raise_TypeError(MP_ERROR_TEXT("xarray type mismatch"));
        }

        switch (parmid & 0x7f) {    // basic argument
            case 0x01:              // float
                farg->type = 0;
                farg->fac.f = (double)mp_obj_get_float(args[j++]);
                break;
            case 0x02:              // int
                farg->type = 1;
                farg->fac.i.l = mp_obj_get_int(args[j++]);
                break;
            case 0x04:              // char
                farg->type = 2;
                farg->fac.i.l = mp_obj_get_int(args[j++]) & 0xff;
                break;
            case 0x08:              // str
                farg->type = 3;
                farg->fac.i.l = (uint32_t)mp_obj_str_get_str(args[j++]);
                break;
            default:
                mp_raise_TypeError(MP_ERROR_TEXT("arguments not supported"));
        }
    }
    parmid = self->parmtbl[i];

#ifdef XFNC_DEBUG
    for (i = 0; i < self->fncarg_sz / 2; i++) {
        printf(" %04x", fncarg[i]);
    }
    printf("\n");
#endif

    mp_int_t result;
    const char *errmsg;
    x68k_fnc_arg_t *resval = NULL;

    __asm volatile (
        "movel %3,%%d1\n"
        "moveml %%d2-%%d7/%%a2-%%a6,%%sp@-\n"
        "subal %%d1,%%sp\n"
        "lsrl #1,%%d1\n"
        "moveal %%sp,%%a0\n"
        "bras 2f\n"
        "1:\n"
        "movew %4@+,%%a0@+\n"
        "2:\n"
        "dbra %%d1,1b\n"
        "jsr %5@\n"
        "moveql #0,%%d1\n"
        "movew %%sp@+,%%d1\n"
        "lsll #1,%%d1\n"
        "addal %%d1,%%sp\n"
        "lsll #2,%%d1\n"
        "addal %%d1,%%sp\n"
        "moveml %%sp@+,%%d2-%%d7/%%a2-%%a6\n"
        "movel %%d0,%0\n"
        "moveal %%a0,%1\n"
        "moveal %%a1,%2\n"
        : "=d"(result), "=a"(resval), "=a"(errmsg)
        : "d"(self->fncarg_sz), "a"(fncarg), "a"(self->fncentry)
        : "%%d0","%%d1","%%a0","%%a1", "memory"
    );

#ifdef XFNC_DEBUG
    printf("result = 0x%04x %ld %08lx\n", parmid, result, resval->fac.i.l);
#endif

    if (result != 0) {
        mp_raise_ValueError(errmsg);
    }

    // convert return value
    if (parmid == 0x8000) {             // float
        return mp_obj_new_float_from_d(resval->fac.f);
    } else if (parmid == 0x8001) {      // int
        return mp_obj_new_int(resval->fac.i.l);
    } else if (parmid == 0x8003) {      // str
        return mp_obj_new_str((char *)resval->fac.i.l, strlen((char *)resval->fac.i.l));
    }
    return mp_const_none;
}

STATIC mp_obj_t x68k_loadfnc(size_t n_args, const mp_obj_t *args) {
    const char *fname = mp_obj_str_get_str(args[0]);
    bool loadhigh = true;
    int res;

    if (n_args > 1) {
        loadhigh = mp_obj_get_int(args[1]);
    }

    // Allocate max memory block
    size_t maxsize = (int)_dos_malloc(0x7fffffff) & 0x00ffffff;
    void *blocktop = _dos_malloc(maxsize);
    void *blockend = ((void **)blocktop)[-2];

    // Load FNC file as .X type
    res = _dos_exec2(3, (const char *)(0x03000000 | (uint32_t)fname),
                     blocktop, blockend);
    if (res < 0) {
        _dos_mfree(blocktop);
        mp_raise_OSError(ENOENT);
    }

    if (!loadhigh) {
        _dos_setblock(blocktop, res);
    } else {
        _dos_mfree(blocktop);

        // Allocate required memory block at the bottom
        blocktop = _dos_malloc2(2, res);
        blockend = ((void **)blocktop)[-2];

        // Reload FNC file at the bottom
        res = _dos_exec2(3, (const char *)(0x03000000 | (uint32_t)fname),
                         blocktop, blockend);
        if (res < 0) {
            _dos_mfree(blocktop);
            mp_raise_OSError(ENOENT);
        }
    }

    // Read FNC file information table

    x68k_fnc_info_t *info = (x68k_fnc_info_t *)blocktop;
    const char *tokentbl;
    size_t tokentbl_len;
    bool loaded = false;

    // Get the size of token table
    tokentbl = info->tokentbl;
    while (*tokentbl != '\0') {
        tokentbl = tokentbl + strlen(tokentbl) + 1;
    }
    tokentbl_len = tokentbl - info->tokentbl + 1;

    // Confirm whether the token table already exists
    x68k_fnc_info_t *ninfo;
    for (ninfo = xfnc_list; ninfo != NULL; ninfo = ninfo->next) {
        if (memcmp(info->tokentbl, ninfo->tokentbl, tokentbl_len) == 0) {
            // Free FNC file and use existing file on the memory
            _dos_mfree(blocktop);
            info = ninfo;
            loaded = true;
            break;
        }
    }

    int nfnc = 0;
    tokentbl = info->tokentbl;

    // Register functions in the FNC file
    while (*tokentbl != '\0') {
#ifdef XFNC_DEBUG
        const uint16_t *pp = info->parmtbl[nfnc];
        printf("%2d: %-10s\t0x%08x", nfnc, tokentbl, (int)info->entrytbl[nfnc]);
        do {
            printf(" %04x", *pp);
        } while (!(*pp++ & 0x8000));
        printf("\n");
#endif
        // Allocate function entry structure

        mp_obj_fun_x68kfnc_t *fnc = m_malloc(sizeof(*fnc));
        fnc->instr.op0 = 0x23df;                // 1: move.l (sp)+,x68kfnc_t_addr
        fnc->instr.op0addr = (uint32_t)&x68kfnc_t_addr;
        fnc->instr.op1 = 0x4ef9;                //    jmp x68k_callfncentry
        fnc->instr.op1addr = (uint32_t)&x68k_callfncentry;
        fnc->instr.op2 = 0x61f2;                //    bsr 1b

        fnc->fncobj = x68k_fnc_obj;
        fnc->fncobj.fun.var = (mp_fun_var_t)&fnc->instr.op2;
        fnc->fncentry = info->entrytbl[nfnc];
        fnc->parmtbl = info->parmtbl[nfnc];

        // Count the # of function arguments
        fnc->n_fncargs = 0;
        fnc->n_optargs = 0;
        while (1) {
            uint16_t parmid = fnc->parmtbl[fnc->n_fncargs];
            if (parmid & 0x8000) {      // Return type
                break;
            }
            if (parmid & 0x80) {        // Optional argument
                fnc->n_optargs++;
            }
            fnc->n_fncargs++;
        }
        fnc->fncarg_sz = sizeof(uint16_t) + sizeof(x68k_fnc_arg_t) * fnc->n_fncargs;

        // Register new function object
        mp_store_global(qstr_from_str(tokentbl), MP_OBJ_FROM_PTR(&fnc->fncobj));

        tokentbl = tokentbl + strlen(tokentbl) + 1;
        nfnc++;
    }

    if (!loaded) {
        // Call start entries only once
        info->entry_boot();
        info->entry_run();

        info->next = xfnc_list;
        xfnc_list = info;
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_loadfnc_obj, 1, 2, x68k_loadfnc);

void x68k_freefnc(void)
{
    x68k_fnc_info_t *info;
    x68k_fnc_info_t *next;

    // Call all end entries
    for (info = xfnc_list; info != NULL; info = next) {
        next = info->next;
        info->entry_end();
    }

    // Call all exit entries
    for (info = xfnc_list; info != NULL; info = next) {
        next = info->next;
        info->entry_exit();
        _dos_mfree(info);
    }
    xfnc_list = NULL;
}
