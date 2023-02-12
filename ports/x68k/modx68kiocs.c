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

STATIC mp_obj_t x68k_iocs(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_d0, ARG_d1, ARG_d2, ARG_d3, ARG_d4, ARG_d5,
           ARG_a1, ARG_a2, ARG_a1w, ARG_a2w, ARG_rd, ARG_ra };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_d0,   MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_d1,   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_d2,   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_d3,   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_d4,   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_d5,   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
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
        if (i == ARG_d0) {
            regs[0] = args[i].u_int;
        } else if (i <= ARG_a2w) {
            int r = (i < ARG_a1w) ? i : i - (ARG_a1w - ARG_a1);
            if (args[i].u_obj != MP_OBJ_NULL) {
                if (mp_obj_is_int(args[i].u_obj)) {
                    regs[r] = mp_obj_get_int(args[i].u_obj);
                } else {
                    mp_buffer_info_t bufinfo;
                    int flags;
                    if (i <= ARG_a2) {
                        flags = MP_BUFFER_READ;
                    } else {
                        flags = MP_BUFFER_RW;
                    }
                    mp_get_buffer_raise(args[i].u_obj, &bufinfo, flags);
                    if (i <= ARG_d5) {
                        regs[r] = *(mp_int_t *)bufinfo.buf;
                    } else {
                        regs[r] = (mp_int_t)bufinfo.buf;
                    }
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

MP_DEFINE_CONST_FUN_OBJ_KW(x68k_iocs_obj, 0, x68k_iocs);

/****************************************************************************/

STATIC const mp_rom_map_elem_t x68k_i_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_B_KEYINP),    MP_ROM_INT(0x00) },
    { MP_ROM_QSTR(MP_QSTR_B_KEYSNS),    MP_ROM_INT(0x01) },
    { MP_ROM_QSTR(MP_QSTR_B_SFTSNS),    MP_ROM_INT(0x02) },
    { MP_ROM_QSTR(MP_QSTR_BITSNS),      MP_ROM_INT(0x04) },
    { MP_ROM_QSTR(MP_QSTR_SKEYSET),     MP_ROM_INT(0x05) },
    { MP_ROM_QSTR(MP_QSTR_LEDCTRL),     MP_ROM_INT(0x06) },
    { MP_ROM_QSTR(MP_QSTR_LEDSET),      MP_ROM_INT(0x07) },
    { MP_ROM_QSTR(MP_QSTR_KEYDLY),      MP_ROM_INT(0x08) },
    { MP_ROM_QSTR(MP_QSTR_KEYREP),      MP_ROM_INT(0x09) },
    { MP_ROM_QSTR(MP_QSTR_OPT2TVON),    MP_ROM_INT(0x0a) },
    { MP_ROM_QSTR(MP_QSTR_OPT2TVOFF),   MP_ROM_INT(0x0b) },
    { MP_ROM_QSTR(MP_QSTR_TVCTRL),      MP_ROM_INT(0x0c) },
    { MP_ROM_QSTR(MP_QSTR_LEDMOD),      MP_ROM_INT(0x0d) },
    { MP_ROM_QSTR(MP_QSTR_TGUSEMD),     MP_ROM_INT(0x0e) },
    { MP_ROM_QSTR(MP_QSTR_DEFCHR),      MP_ROM_INT(0x0f) },
    { MP_ROM_QSTR(MP_QSTR_CRTMOD),      MP_ROM_INT(0x10) },
    { MP_ROM_QSTR(MP_QSTR_CONTRAST),    MP_ROM_INT(0x11) },
    { MP_ROM_QSTR(MP_QSTR_HSVTORGB),    MP_ROM_INT(0x12) },
    { MP_ROM_QSTR(MP_QSTR_TPALET),      MP_ROM_INT(0x13) },
    { MP_ROM_QSTR(MP_QSTR_TPALET2),     MP_ROM_INT(0x14) },
    { MP_ROM_QSTR(MP_QSTR_TCOLOR),      MP_ROM_INT(0x15) },
    { MP_ROM_QSTR(MP_QSTR_VRAMGET),     MP_ROM_INT(0x17) },
    { MP_ROM_QSTR(MP_QSTR_VRAMPUT),     MP_ROM_INT(0x18) },
    { MP_ROM_QSTR(MP_QSTR_FNTGET),      MP_ROM_INT(0x19) },
    { MP_ROM_QSTR(MP_QSTR_TEXTGET),     MP_ROM_INT(0x1a) },
    { MP_ROM_QSTR(MP_QSTR_TEXTPUT),     MP_ROM_INT(0x1b) },
    { MP_ROM_QSTR(MP_QSTR_CLIPPUT),     MP_ROM_INT(0x1c) },
    { MP_ROM_QSTR(MP_QSTR_SCROLL),      MP_ROM_INT(0x1d) },
    { MP_ROM_QSTR(MP_QSTR_B_CURON),     MP_ROM_INT(0x1e) },
    { MP_ROM_QSTR(MP_QSTR_B_CUROFF),    MP_ROM_INT(0x1f) },
    { MP_ROM_QSTR(MP_QSTR_B_PUTC),      MP_ROM_INT(0x20) },
    { MP_ROM_QSTR(MP_QSTR_B_PRINT),     MP_ROM_INT(0x21) },
    { MP_ROM_QSTR(MP_QSTR_B_COLOR),     MP_ROM_INT(0x22) },
    { MP_ROM_QSTR(MP_QSTR_B_LOCATE),    MP_ROM_INT(0x23) },
    { MP_ROM_QSTR(MP_QSTR_B_DOWN_S),    MP_ROM_INT(0x24) },
    { MP_ROM_QSTR(MP_QSTR_B_UP_S),      MP_ROM_INT(0x25) },
    { MP_ROM_QSTR(MP_QSTR_B_UP),        MP_ROM_INT(0x26) },
    { MP_ROM_QSTR(MP_QSTR_B_DOWN),      MP_ROM_INT(0x27) },
    { MP_ROM_QSTR(MP_QSTR_B_RIGHT),     MP_ROM_INT(0x28) },
    { MP_ROM_QSTR(MP_QSTR_B_LEFT),      MP_ROM_INT(0x29) },
    { MP_ROM_QSTR(MP_QSTR_B_CLR_ST),    MP_ROM_INT(0x2a) },
    { MP_ROM_QSTR(MP_QSTR_B_ERA_ST),    MP_ROM_INT(0x2b) },
    { MP_ROM_QSTR(MP_QSTR_B_INS),       MP_ROM_INT(0x2c) },
    { MP_ROM_QSTR(MP_QSTR_B_DEL),       MP_ROM_INT(0x2d) },
    { MP_ROM_QSTR(MP_QSTR_B_CONSOL),    MP_ROM_INT(0x2e) },
    { MP_ROM_QSTR(MP_QSTR_B_PUTMES),    MP_ROM_INT(0x2f) },
    { MP_ROM_QSTR(MP_QSTR_SET232C),     MP_ROM_INT(0x30) },
    { MP_ROM_QSTR(MP_QSTR_LOF232C),     MP_ROM_INT(0x31) },
    { MP_ROM_QSTR(MP_QSTR_INP232C),     MP_ROM_INT(0x32) },
    { MP_ROM_QSTR(MP_QSTR_ISNS232C),    MP_ROM_INT(0x33) },
    { MP_ROM_QSTR(MP_QSTR_OSNS232C),    MP_ROM_INT(0x34) },
    { MP_ROM_QSTR(MP_QSTR_OUT232C),     MP_ROM_INT(0x35) },
    { MP_ROM_QSTR(MP_QSTR_MS_VCS),      MP_ROM_INT(0x36) },
    { MP_ROM_QSTR(MP_QSTR_EXESC),       MP_ROM_INT(0x37) },
    { MP_ROM_QSTR(MP_QSTR_CHR_ADR),     MP_ROM_INT(0x38) },
    { MP_ROM_QSTR(MP_QSTR_SETBEEP),     MP_ROM_INT(0x39) },
    { MP_ROM_QSTR(MP_QSTR_SETPRN),      MP_ROM_INT(0x3a) },
    { MP_ROM_QSTR(MP_QSTR_JOYGET),      MP_ROM_INT(0x3b) },
    { MP_ROM_QSTR(MP_QSTR_INIT_PRN),    MP_ROM_INT(0x3c) },
    { MP_ROM_QSTR(MP_QSTR_SNSPRN),      MP_ROM_INT(0x3d) },
    { MP_ROM_QSTR(MP_QSTR_OUTLPT),      MP_ROM_INT(0x3e) },
    { MP_ROM_QSTR(MP_QSTR_OUTPRN),      MP_ROM_INT(0x3f) },
    { MP_ROM_QSTR(MP_QSTR_B_SEEK),      MP_ROM_INT(0x40) },
    { MP_ROM_QSTR(MP_QSTR_B_VERIFY),    MP_ROM_INT(0x41) },
    { MP_ROM_QSTR(MP_QSTR_B_READDI),    MP_ROM_INT(0x42) },
    { MP_ROM_QSTR(MP_QSTR_B_DSKINI),    MP_ROM_INT(0x43) },
    { MP_ROM_QSTR(MP_QSTR_B_DRVSNS),    MP_ROM_INT(0x44) },
    { MP_ROM_QSTR(MP_QSTR_B_WRITE),     MP_ROM_INT(0x45) },
    { MP_ROM_QSTR(MP_QSTR_B_READ),      MP_ROM_INT(0x46) },
    { MP_ROM_QSTR(MP_QSTR_B_RECALI),    MP_ROM_INT(0x47) },
    { MP_ROM_QSTR(MP_QSTR_B_ASSIGN),    MP_ROM_INT(0x48) },
    { MP_ROM_QSTR(MP_QSTR_B_WRITED),    MP_ROM_INT(0x49) },
    { MP_ROM_QSTR(MP_QSTR_B_READID),    MP_ROM_INT(0x4a) },
    { MP_ROM_QSTR(MP_QSTR_B_BADFMT),    MP_ROM_INT(0x4b) },
    { MP_ROM_QSTR(MP_QSTR_B_READDL),    MP_ROM_INT(0x4c) },
    { MP_ROM_QSTR(MP_QSTR_B_FORMAT),    MP_ROM_INT(0x4d) },
    { MP_ROM_QSTR(MP_QSTR_B_DRVCHK),    MP_ROM_INT(0x4e) },
    { MP_ROM_QSTR(MP_QSTR_B_EJECT),     MP_ROM_INT(0x4f) },
    { MP_ROM_QSTR(MP_QSTR_DATEBCD),     MP_ROM_INT(0x50) },
    { MP_ROM_QSTR(MP_QSTR_DATESET),     MP_ROM_INT(0x51) },
    { MP_ROM_QSTR(MP_QSTR_TIMEBCD),     MP_ROM_INT(0x52) },
    { MP_ROM_QSTR(MP_QSTR_TIMESET),     MP_ROM_INT(0x53) },
    { MP_ROM_QSTR(MP_QSTR_DATEGET),     MP_ROM_INT(0x54) },
    { MP_ROM_QSTR(MP_QSTR_DATEBIN),     MP_ROM_INT(0x55) },
    { MP_ROM_QSTR(MP_QSTR_TIMEGET),     MP_ROM_INT(0x56) },
    { MP_ROM_QSTR(MP_QSTR_TIMEBIN),     MP_ROM_INT(0x57) },
    { MP_ROM_QSTR(MP_QSTR_DATECNV),     MP_ROM_INT(0x58) },
    { MP_ROM_QSTR(MP_QSTR_TIMECNV),     MP_ROM_INT(0x59) },
    { MP_ROM_QSTR(MP_QSTR_DATEASC),     MP_ROM_INT(0x5a) },
    { MP_ROM_QSTR(MP_QSTR_TIMEASC),     MP_ROM_INT(0x5b) },
    { MP_ROM_QSTR(MP_QSTR_DAYASC),      MP_ROM_INT(0x5c) },
    { MP_ROM_QSTR(MP_QSTR_ALARMMOD),    MP_ROM_INT(0x5d) },
    { MP_ROM_QSTR(MP_QSTR_ALARMSET),    MP_ROM_INT(0x5e) },
    { MP_ROM_QSTR(MP_QSTR_ALARMGET),    MP_ROM_INT(0x5f) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMOUT),    MP_ROM_INT(0x60) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMINP),    MP_ROM_INT(0x61) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMAOT),    MP_ROM_INT(0x62) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMAIN),    MP_ROM_INT(0x63) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMLOT),    MP_ROM_INT(0x64) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMLIN),    MP_ROM_INT(0x65) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMSNS),    MP_ROM_INT(0x66) },
    { MP_ROM_QSTR(MP_QSTR_ADPCMMOD),    MP_ROM_INT(0x67) },
    { MP_ROM_QSTR(MP_QSTR_OPMSET),      MP_ROM_INT(0x68) },
    { MP_ROM_QSTR(MP_QSTR_OPMSNS),      MP_ROM_INT(0x69) },
    { MP_ROM_QSTR(MP_QSTR_OPMINTST),    MP_ROM_INT(0x6a) },
    { MP_ROM_QSTR(MP_QSTR_TIMERDST),    MP_ROM_INT(0x6b) },
    { MP_ROM_QSTR(MP_QSTR_VDISPST),     MP_ROM_INT(0x6c) },
    { MP_ROM_QSTR(MP_QSTR_CRTCRAS),     MP_ROM_INT(0x6d) },
    { MP_ROM_QSTR(MP_QSTR_HSYNCST),     MP_ROM_INT(0x6e) },
    { MP_ROM_QSTR(MP_QSTR_PRNINTST),    MP_ROM_INT(0x6f) },
    { MP_ROM_QSTR(MP_QSTR_MS_INIT),     MP_ROM_INT(0x70) },
    { MP_ROM_QSTR(MP_QSTR_MS_CURON),    MP_ROM_INT(0x71) },
    { MP_ROM_QSTR(MP_QSTR_MS_CUROF),    MP_ROM_INT(0x72) },
    { MP_ROM_QSTR(MP_QSTR_MS_STAT),     MP_ROM_INT(0x73) },
    { MP_ROM_QSTR(MP_QSTR_MS_GETDT),    MP_ROM_INT(0x74) },
    { MP_ROM_QSTR(MP_QSTR_MS_CURGT),    MP_ROM_INT(0x75) },
    { MP_ROM_QSTR(MP_QSTR_MS_CURST),    MP_ROM_INT(0x76) },
    { MP_ROM_QSTR(MP_QSTR_MS_LIMIT),    MP_ROM_INT(0x77) },
    { MP_ROM_QSTR(MP_QSTR_MS_OFFTM),    MP_ROM_INT(0x78) },
    { MP_ROM_QSTR(MP_QSTR_MS_ONTM),     MP_ROM_INT(0x79) },
    { MP_ROM_QSTR(MP_QSTR_MS_PATST),    MP_ROM_INT(0x7a) },
    { MP_ROM_QSTR(MP_QSTR_MS_SEL),      MP_ROM_INT(0x7b) },
    { MP_ROM_QSTR(MP_QSTR_MS_SEL2),     MP_ROM_INT(0x7c) },
    { MP_ROM_QSTR(MP_QSTR_SKEY_MOD),    MP_ROM_INT(0x7d) },
    { MP_ROM_QSTR(MP_QSTR_DENSNS),      MP_ROM_INT(0x7e) },
    { MP_ROM_QSTR(MP_QSTR_ONTIME),      MP_ROM_INT(0x7f) },
    { MP_ROM_QSTR(MP_QSTR_B_INTVCS),    MP_ROM_INT(0x80) },
    { MP_ROM_QSTR(MP_QSTR_B_SUPER),     MP_ROM_INT(0x81) },
    { MP_ROM_QSTR(MP_QSTR_B_BPEEK),     MP_ROM_INT(0x82) },
    { MP_ROM_QSTR(MP_QSTR_B_WPEEK),     MP_ROM_INT(0x83) },
    { MP_ROM_QSTR(MP_QSTR_B_LPEEK),     MP_ROM_INT(0x84) },
    { MP_ROM_QSTR(MP_QSTR_B_MEMSTR),    MP_ROM_INT(0x85) },
    { MP_ROM_QSTR(MP_QSTR_B_BPOKE),     MP_ROM_INT(0x86) },
    { MP_ROM_QSTR(MP_QSTR_B_WPOKE),     MP_ROM_INT(0x87) },
    { MP_ROM_QSTR(MP_QSTR_B_LPOKE),     MP_ROM_INT(0x88) },
    { MP_ROM_QSTR(MP_QSTR_B_MEMSET),    MP_ROM_INT(0x89) },
    { MP_ROM_QSTR(MP_QSTR_DMAMOVE),     MP_ROM_INT(0x8a) },
    { MP_ROM_QSTR(MP_QSTR_DMAMOV_A),    MP_ROM_INT(0x8b) },
    { MP_ROM_QSTR(MP_QSTR_DMAMOV_L),    MP_ROM_INT(0x8c) },
    { MP_ROM_QSTR(MP_QSTR_DMAMODE),     MP_ROM_INT(0x8d) },
    { MP_ROM_QSTR(MP_QSTR_BOOTINF),     MP_ROM_INT(0x8e) },
    { MP_ROM_QSTR(MP_QSTR_ROMVER),      MP_ROM_INT(0x8f) },
    { MP_ROM_QSTR(MP_QSTR_G_CLR_ON),    MP_ROM_INT(0x90) },
    { MP_ROM_QSTR(MP_QSTR_G_MOD),       MP_ROM_INT(0x91) },
    { MP_ROM_QSTR(MP_QSTR_PRIORITY),    MP_ROM_INT(0x92) },
    { MP_ROM_QSTR(MP_QSTR_CRTMOD2),     MP_ROM_INT(0x93) },
    { MP_ROM_QSTR(MP_QSTR_GPALET),      MP_ROM_INT(0x94) },
    { MP_ROM_QSTR(MP_QSTR_PENCOLOR),    MP_ROM_INT(0x95) },
    { MP_ROM_QSTR(MP_QSTR_SET_PAGE),    MP_ROM_INT(0x96) },
    { MP_ROM_QSTR(MP_QSTR_GGET),        MP_ROM_INT(0x97) },
    { MP_ROM_QSTR(MP_QSTR_MASK_GPUT),   MP_ROM_INT(0x98) },
    { MP_ROM_QSTR(MP_QSTR_GPUT),        MP_ROM_INT(0x99) },
    { MP_ROM_QSTR(MP_QSTR_GPTRN),       MP_ROM_INT(0x9a) },
    { MP_ROM_QSTR(MP_QSTR_BK_GPTRN),    MP_ROM_INT(0x9b) },
    { MP_ROM_QSTR(MP_QSTR_X_GPTRN),     MP_ROM_INT(0x9c) },
    { MP_ROM_QSTR(MP_QSTR_SFTJIS),      MP_ROM_INT(0xa0) },
    { MP_ROM_QSTR(MP_QSTR_JISSFT),      MP_ROM_INT(0xa1) },
    { MP_ROM_QSTR(MP_QSTR_AKCONV),      MP_ROM_INT(0xa2) },
    { MP_ROM_QSTR(MP_QSTR_RMACNV),      MP_ROM_INT(0xa3) },
    { MP_ROM_QSTR(MP_QSTR_DAKJOB),      MP_ROM_INT(0xa4) },
    { MP_ROM_QSTR(MP_QSTR_HANJOB),      MP_ROM_INT(0xa5) },
    { MP_ROM_QSTR(MP_QSTR_SYS_STAT),    MP_ROM_INT(0xac) },
    { MP_ROM_QSTR(MP_QSTR_B_CONMOD),    MP_ROM_INT(0xad) },
    { MP_ROM_QSTR(MP_QSTR_OS_CURON),    MP_ROM_INT(0xae) },
    { MP_ROM_QSTR(MP_QSTR_OS_CUROF),    MP_ROM_INT(0xaf) },
    { MP_ROM_QSTR(MP_QSTR_DRAWMODE),    MP_ROM_INT(0xb0) },
    { MP_ROM_QSTR(MP_QSTR_APAGE),       MP_ROM_INT(0xb1) },
    { MP_ROM_QSTR(MP_QSTR_VPAGE),       MP_ROM_INT(0xb2) },
    { MP_ROM_QSTR(MP_QSTR_HOME),        MP_ROM_INT(0xb3) },
    { MP_ROM_QSTR(MP_QSTR_WINDOW),      MP_ROM_INT(0xb4) },
    { MP_ROM_QSTR(MP_QSTR_WIPE),        MP_ROM_INT(0xb5) },
    { MP_ROM_QSTR(MP_QSTR_PSET),        MP_ROM_INT(0xb6) },
    { MP_ROM_QSTR(MP_QSTR_POINT),       MP_ROM_INT(0xb7) },
    { MP_ROM_QSTR(MP_QSTR_LINE),        MP_ROM_INT(0xb8) },
    { MP_ROM_QSTR(MP_QSTR_BOX),         MP_ROM_INT(0xb9) },
    { MP_ROM_QSTR(MP_QSTR_FILL),        MP_ROM_INT(0xba) },
    { MP_ROM_QSTR(MP_QSTR_CIRCLE),      MP_ROM_INT(0xbb) },
    { MP_ROM_QSTR(MP_QSTR_PAINT),       MP_ROM_INT(0xbc) },
    { MP_ROM_QSTR(MP_QSTR_SYMBOL),      MP_ROM_INT(0xbd) },
    { MP_ROM_QSTR(MP_QSTR_GETGRM),      MP_ROM_INT(0xbe) },
    { MP_ROM_QSTR(MP_QSTR_PUTGRM),      MP_ROM_INT(0xbf) },
    { MP_ROM_QSTR(MP_QSTR_SP_INIT),     MP_ROM_INT(0xc0) },
    { MP_ROM_QSTR(MP_QSTR_SP_ON),       MP_ROM_INT(0xc1) },
    { MP_ROM_QSTR(MP_QSTR_SP_OFF),      MP_ROM_INT(0xc2) },
    { MP_ROM_QSTR(MP_QSTR_SP_CGCLR),    MP_ROM_INT(0xc3) },
    { MP_ROM_QSTR(MP_QSTR_SP_DEFCG),    MP_ROM_INT(0xc4) },
    { MP_ROM_QSTR(MP_QSTR_SP_GTPCG),    MP_ROM_INT(0xc5) },
    { MP_ROM_QSTR(MP_QSTR_SP_REGST),    MP_ROM_INT(0xc6) },
    { MP_ROM_QSTR(MP_QSTR_SP_REGGT),    MP_ROM_INT(0xc7) },
    { MP_ROM_QSTR(MP_QSTR_BGSCRLST),    MP_ROM_INT(0xc8) },
    { MP_ROM_QSTR(MP_QSTR_BGSCRLGT),    MP_ROM_INT(0xc9) },
    { MP_ROM_QSTR(MP_QSTR_BGCTRLST),    MP_ROM_INT(0xca) },
    { MP_ROM_QSTR(MP_QSTR_BGCTRLGT),    MP_ROM_INT(0xcb) },
    { MP_ROM_QSTR(MP_QSTR_BGTEXTCL),    MP_ROM_INT(0xcc) },
    { MP_ROM_QSTR(MP_QSTR_BGTEXTST),    MP_ROM_INT(0xcd) },
    { MP_ROM_QSTR(MP_QSTR_BGTEXTGT),    MP_ROM_INT(0xce) },
    { MP_ROM_QSTR(MP_QSTR_SPALET),      MP_ROM_INT(0xcf) },
    { MP_ROM_QSTR(MP_QSTR_TXXLINE),     MP_ROM_INT(0xd3) },
    { MP_ROM_QSTR(MP_QSTR_TXYLINE),     MP_ROM_INT(0xd4) },
    { MP_ROM_QSTR(MP_QSTR_TXLINE),      MP_ROM_INT(0xd5) },
    { MP_ROM_QSTR(MP_QSTR_TXBOX),       MP_ROM_INT(0xd6) },
    { MP_ROM_QSTR(MP_QSTR_TXFILL),      MP_ROM_INT(0xd7) },
    { MP_ROM_QSTR(MP_QSTR_TXREV),       MP_ROM_INT(0xd8) },
    { MP_ROM_QSTR(MP_QSTR_TXRASCPY),    MP_ROM_INT(0xdf) },
    { MP_ROM_QSTR(MP_QSTR_OPMDRV),      MP_ROM_INT(0xf0) },
    { MP_ROM_QSTR(MP_QSTR_RSDRV),       MP_ROM_INT(0xf1) },
    { MP_ROM_QSTR(MP_QSTR_SCSIDRV),     MP_ROM_INT(0xf5) },
    { MP_ROM_QSTR(MP_QSTR_ABORTRST),    MP_ROM_INT(0xfd) },
    { MP_ROM_QSTR(MP_QSTR_IPLERR),      MP_ROM_INT(0xfe) },
    { MP_ROM_QSTR(MP_QSTR_ABORTJOB),    MP_ROM_INT(0xff) },
};

STATIC MP_DEFINE_CONST_DICT(x68k_i_locals_dict, x68k_i_locals_dict_table);

const mp_obj_type_t x68k_i_obj_type = {
    { &mp_type_type },
    .name = MP_QSTR_i,
    .locals_dict = (mp_obj_dict_t *)&x68k_i_locals_dict,
};
