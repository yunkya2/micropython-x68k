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

#include "py/runtime.h"
#include "py/obj.h"
#include "modx68k.h"

mp_obj_t x68k_dos(size_t n_args, const mp_obj_t *args) {
    mp_int_t result;
    uint32_t doscall;
    mp_buffer_info_t bufinfo;

    /* DOSCALL _XXXX ; JMP (a1)*/
    doscall = (mp_obj_get_int(args[0]) << 16) | 0x4ed1;

    bufinfo.len = 0;
    if (n_args > 1) {
        mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);
    }

    __asm volatile (
        "subal %1,%%sp\n"
        "moveal %%sp,%%a0\n"
        "bras 2f\n"
        "1:\n"
        "move.w %3@+,%%a0@+\n"
        "2:\n"
        "dbra %2,1b\n"
        "leal %%pc@(3f),%%a1\n"
        "jmp %4@\n"
        "3:\n"
        "addal %1,%%sp\n"
        "movel %%d0,%0\n"
        : "=d"(result)
        : "d"(bufinfo.len), "d"(bufinfo.len / 2),
          "a"(bufinfo.buf), "a"(&doscall)
        : "%%d0","%%a0","%%a1", "memory"
    );

    return mp_obj_new_int(result);
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(x68k_dos_obj, 1, 2, x68k_dos);

/****************************************************************************/

STATIC const mp_rom_map_elem_t x68k_d_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_EXIT),        MP_ROM_INT(0xff00) },
    { MP_ROM_QSTR(MP_QSTR_GETCHAR),     MP_ROM_INT(0xff01) },
    { MP_ROM_QSTR(MP_QSTR_PUTCHAR),     MP_ROM_INT(0xff02) },
    { MP_ROM_QSTR(MP_QSTR_COMINP),      MP_ROM_INT(0xff03) },
    { MP_ROM_QSTR(MP_QSTR_COMOUT),      MP_ROM_INT(0xff04) },
    { MP_ROM_QSTR(MP_QSTR_PRNOUT),      MP_ROM_INT(0xff05) },
    { MP_ROM_QSTR(MP_QSTR_INPOUT),      MP_ROM_INT(0xff06) },
    { MP_ROM_QSTR(MP_QSTR_INKEY),       MP_ROM_INT(0xff07) },
    { MP_ROM_QSTR(MP_QSTR_GETC),        MP_ROM_INT(0xff08) },
    { MP_ROM_QSTR(MP_QSTR_PRINT),       MP_ROM_INT(0xff09) },
    { MP_ROM_QSTR(MP_QSTR_GETS),        MP_ROM_INT(0xff0a) },
    { MP_ROM_QSTR(MP_QSTR_KEYSNS),      MP_ROM_INT(0xff0b) },
    { MP_ROM_QSTR(MP_QSTR_KFLUSH),      MP_ROM_INT(0xff0c) },
    { MP_ROM_QSTR(MP_QSTR_FFLUSH),      MP_ROM_INT(0xff0d) },
    { MP_ROM_QSTR(MP_QSTR_CHGDRV),      MP_ROM_INT(0xff0e) },
    { MP_ROM_QSTR(MP_QSTR_CHDRV),       MP_ROM_INT(0xff0e) },
    { MP_ROM_QSTR(MP_QSTR_DRVCTRL),     MP_ROM_INT(0xff0f) },
    { MP_ROM_QSTR(MP_QSTR_CONSNS),      MP_ROM_INT(0xff10) },
    { MP_ROM_QSTR(MP_QSTR_PRNSNS),      MP_ROM_INT(0xff11) },
    { MP_ROM_QSTR(MP_QSTR_CINSNS),      MP_ROM_INT(0xff12) },
    { MP_ROM_QSTR(MP_QSTR_COUTSNS),     MP_ROM_INT(0xff13) },
    { MP_ROM_QSTR(MP_QSTR_FATCHK),      MP_ROM_INT(0xff17) },
    { MP_ROM_QSTR(MP_QSTR_CURDRV),      MP_ROM_INT(0xff19) },
    { MP_ROM_QSTR(MP_QSTR_GETSS),       MP_ROM_INT(0xff1a) },
    { MP_ROM_QSTR(MP_QSTR_FGETC),       MP_ROM_INT(0xff1b) },
    { MP_ROM_QSTR(MP_QSTR_FGETS),       MP_ROM_INT(0xff1c) },
    { MP_ROM_QSTR(MP_QSTR_FPUTC),       MP_ROM_INT(0xff1d) },
    { MP_ROM_QSTR(MP_QSTR_FPUTS),       MP_ROM_INT(0xff1e) },
    { MP_ROM_QSTR(MP_QSTR_ALLCLOSE),    MP_ROM_INT(0xff1f) },
    { MP_ROM_QSTR(MP_QSTR_SUPER),       MP_ROM_INT(0xff20) },
    { MP_ROM_QSTR(MP_QSTR_FNCKEY),      MP_ROM_INT(0xff21) },
    { MP_ROM_QSTR(MP_QSTR_KNJCTRL),     MP_ROM_INT(0xff22) },
    { MP_ROM_QSTR(MP_QSTR_CONCTRL),     MP_ROM_INT(0xff23) },
    { MP_ROM_QSTR(MP_QSTR_KEYCTRL),     MP_ROM_INT(0xff24) },
    { MP_ROM_QSTR(MP_QSTR_INTVCS),      MP_ROM_INT(0xff25) },
    { MP_ROM_QSTR(MP_QSTR_PSPSET),      MP_ROM_INT(0xff26) },
    { MP_ROM_QSTR(MP_QSTR_GETTIM2),     MP_ROM_INT(0xff27) },
    { MP_ROM_QSTR(MP_QSTR_SETTIM2),     MP_ROM_INT(0xff28) },
    { MP_ROM_QSTR(MP_QSTR_NAMESTS),     MP_ROM_INT(0xff29) },
    { MP_ROM_QSTR(MP_QSTR_GETDATE),     MP_ROM_INT(0xff2a) },
    { MP_ROM_QSTR(MP_QSTR_SETDATE),     MP_ROM_INT(0xff2b) },
    { MP_ROM_QSTR(MP_QSTR_GETTIME),     MP_ROM_INT(0xff2c) },
    { MP_ROM_QSTR(MP_QSTR_SETTIME),     MP_ROM_INT(0xff2d) },
    { MP_ROM_QSTR(MP_QSTR_VERIFY),      MP_ROM_INT(0xff2e) },
    { MP_ROM_QSTR(MP_QSTR_DUP0),        MP_ROM_INT(0xff2f) },
    { MP_ROM_QSTR(MP_QSTR_VERNUM),      MP_ROM_INT(0xff30) },
    { MP_ROM_QSTR(MP_QSTR_KEEPPR),      MP_ROM_INT(0xff31) },
    { MP_ROM_QSTR(MP_QSTR_GETDPB),      MP_ROM_INT(0xff32) },
    { MP_ROM_QSTR(MP_QSTR_BREAKCK),     MP_ROM_INT(0xff33) },
    { MP_ROM_QSTR(MP_QSTR_DRVXCHG),     MP_ROM_INT(0xff34) },
    { MP_ROM_QSTR(MP_QSTR_INTVCG),      MP_ROM_INT(0xff35) },
    { MP_ROM_QSTR(MP_QSTR_DSKFRE),      MP_ROM_INT(0xff36) },
    { MP_ROM_QSTR(MP_QSTR_NAMECK),      MP_ROM_INT(0xff37) },
    { MP_ROM_QSTR(MP_QSTR_MKDIR),       MP_ROM_INT(0xff39) },
    { MP_ROM_QSTR(MP_QSTR_RMDIR),       MP_ROM_INT(0xff3a) },
    { MP_ROM_QSTR(MP_QSTR_CHDIR),       MP_ROM_INT(0xff3b) },
    { MP_ROM_QSTR(MP_QSTR_CREATE),      MP_ROM_INT(0xff3c) },
    { MP_ROM_QSTR(MP_QSTR_OPEN),        MP_ROM_INT(0xff3d) },
    { MP_ROM_QSTR(MP_QSTR_CLOSE),       MP_ROM_INT(0xff3e) },
    { MP_ROM_QSTR(MP_QSTR_READ),        MP_ROM_INT(0xff3f) },
    { MP_ROM_QSTR(MP_QSTR_WRITE),       MP_ROM_INT(0xff40) },
    { MP_ROM_QSTR(MP_QSTR_DELETE),      MP_ROM_INT(0xff41) },
    { MP_ROM_QSTR(MP_QSTR_SEEK),        MP_ROM_INT(0xff42) },
    { MP_ROM_QSTR(MP_QSTR_CHMOD),       MP_ROM_INT(0xff43) },
    { MP_ROM_QSTR(MP_QSTR_IOCTRL),      MP_ROM_INT(0xff44) },
    { MP_ROM_QSTR(MP_QSTR_DUP),         MP_ROM_INT(0xff45) },
    { MP_ROM_QSTR(MP_QSTR_DUP2),        MP_ROM_INT(0xff46) },
    { MP_ROM_QSTR(MP_QSTR_CURDIR),      MP_ROM_INT(0xff47) },
    { MP_ROM_QSTR(MP_QSTR_MALLOC),      MP_ROM_INT(0xff48) },
    { MP_ROM_QSTR(MP_QSTR_MFREE),       MP_ROM_INT(0xff49) },
    { MP_ROM_QSTR(MP_QSTR_SETBLOCK),    MP_ROM_INT(0xff4a) },
    { MP_ROM_QSTR(MP_QSTR_EXEC),        MP_ROM_INT(0xff4b) },
    { MP_ROM_QSTR(MP_QSTR_EXIT2),       MP_ROM_INT(0xff4c) },
    { MP_ROM_QSTR(MP_QSTR_WAIT),        MP_ROM_INT(0xff4d) },
    { MP_ROM_QSTR(MP_QSTR_FILES),       MP_ROM_INT(0xff4e) },
    { MP_ROM_QSTR(MP_QSTR_NFILES),      MP_ROM_INT(0xff4f) },
    { MP_ROM_QSTR(MP_QSTR_SETPDB),      MP_ROM_INT(0xff80) },
    { MP_ROM_QSTR(MP_QSTR_GETPDB),      MP_ROM_INT(0xff81) },
    { MP_ROM_QSTR(MP_QSTR_SETENV),      MP_ROM_INT(0xff82) },
    { MP_ROM_QSTR(MP_QSTR_GETENV),      MP_ROM_INT(0xff83) },
    { MP_ROM_QSTR(MP_QSTR_VERIFYG),     MP_ROM_INT(0xff84) },
    { MP_ROM_QSTR(MP_QSTR_COMMON),      MP_ROM_INT(0xff85) },
    { MP_ROM_QSTR(MP_QSTR_RENAME),      MP_ROM_INT(0xff86) },
    { MP_ROM_QSTR(MP_QSTR_FILEDATE),    MP_ROM_INT(0xff87) },
    { MP_ROM_QSTR(MP_QSTR_MALLOC2),     MP_ROM_INT(0xff88) },
    { MP_ROM_QSTR(MP_QSTR_MAKETMP),     MP_ROM_INT(0xff8A) },
    { MP_ROM_QSTR(MP_QSTR_NEWFILE),     MP_ROM_INT(0xff8B) },
    { MP_ROM_QSTR(MP_QSTR_LOCK),        MP_ROM_INT(0xff8C) },
    { MP_ROM_QSTR(MP_QSTR_ASSIGN),      MP_ROM_INT(0xff8F) },
    { MP_ROM_QSTR(MP_QSTR_FFLUSH_SET),  MP_ROM_INT(0xffAA) },
    { MP_ROM_QSTR(MP_QSTR_OS_PATCH),    MP_ROM_INT(0xffAB) },
    { MP_ROM_QSTR(MP_QSTR_GET_FCB_ADR), MP_ROM_INT(0xffAC) },
    { MP_ROM_QSTR(MP_QSTR_S_MALLOC),    MP_ROM_INT(0xffAD) },
    { MP_ROM_QSTR(MP_QSTR_S_MFREE),     MP_ROM_INT(0xffAE) },
    { MP_ROM_QSTR(MP_QSTR_S_PROCESS),   MP_ROM_INT(0xffAF) },
    { MP_ROM_QSTR(MP_QSTR_EXITVC),      MP_ROM_INT(0xfff0) },
    { MP_ROM_QSTR(MP_QSTR_CTRLVC),      MP_ROM_INT(0xfff1) },
    { MP_ROM_QSTR(MP_QSTR_ERRJVC),      MP_ROM_INT(0xfff2) },
    { MP_ROM_QSTR(MP_QSTR_DISKRED),     MP_ROM_INT(0xfff3) },
    { MP_ROM_QSTR(MP_QSTR_DISKWRT),     MP_ROM_INT(0xfff4) },
    { MP_ROM_QSTR(MP_QSTR_INDOSFLG),    MP_ROM_INT(0xfff5) },
    { MP_ROM_QSTR(MP_QSTR_SUPER_JSR),   MP_ROM_INT(0xfff6) },
    { MP_ROM_QSTR(MP_QSTR_BUS_ERR),     MP_ROM_INT(0xfff7) },
    { MP_ROM_QSTR(MP_QSTR_OPEN_PR),     MP_ROM_INT(0xfff8) },
    { MP_ROM_QSTR(MP_QSTR_KILL_PR),     MP_ROM_INT(0xfff9) },
    { MP_ROM_QSTR(MP_QSTR_GET_PR),      MP_ROM_INT(0xfffa) },
    { MP_ROM_QSTR(MP_QSTR_SUSPEND),     MP_ROM_INT(0xfffb) },
    { MP_ROM_QSTR(MP_QSTR_SLEEP_PR),    MP_ROM_INT(0xfffc) },
    { MP_ROM_QSTR(MP_QSTR_SEND_PR),     MP_ROM_INT(0xfffd) },
    { MP_ROM_QSTR(MP_QSTR_TIME_PR),     MP_ROM_INT(0xfffe) },
    { MP_ROM_QSTR(MP_QSTR_CHANGE_PR),   MP_ROM_INT(0xffff) },
};

STATIC MP_DEFINE_CONST_DICT(x68k_d_locals_dict, x68k_d_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    x68k_d_obj_type,
    MP_QSTR_d,
    MP_TYPE_FLAG_NONE,
    locals_dict, &x68k_d_locals_dict
    );
