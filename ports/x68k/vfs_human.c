/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2018 Damien P. George
 * Copyright (c) 2023      Yuichi Nakamura
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
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/mpthread.h"
#include "extmod/vfs.h"
#include "vfs_human.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <x68k/dos.h>


typedef struct _mp_obj_vfs_human_t {
    mp_obj_base_t base;
    vstr_t root;
    size_t root_len;
    bool readonly;
} mp_obj_vfs_human_t;

STATIC const char *vfs_human_get_path_str(mp_obj_vfs_human_t *self, mp_obj_t path) {
    if (self->root_len == 0) {
        return mp_obj_str_get_str(path);
    } else {
        self->root.len = self->root_len;
        vstr_add_str(&self->root, mp_obj_str_get_str(path));
        return vstr_null_terminated_str(&self->root);
    }
}

STATIC mp_obj_t vfs_human_get_path_obj(mp_obj_vfs_human_t *self, mp_obj_t path) {
    if (self->root_len == 0) {
        return path;
    } else {
        self->root.len = self->root_len;
        vstr_add_str(&self->root, mp_obj_str_get_str(path));
        return mp_obj_new_str(self->root.buf, self->root.len);
    }
}

STATIC mp_obj_t vfs_human_fun1_helper(mp_obj_t self_in, mp_obj_t path_in, int (*f)(const char *)) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    int ret = f(vfs_human_get_path_str(self, path_in));
    if (ret < 0) {
        mp_raise_OSError(errno);
    }
    return mp_const_none;
}

STATIC mp_import_stat_t mp_vfs_human_import_stat(void *self_in, const char *path) {
    mp_obj_vfs_human_t *self = self_in;
    if (self->root_len != 0) {
        self->root.len = self->root_len;
        vstr_add_str(&self->root, path);
        path = vstr_null_terminated_str(&self->root);
    }
    struct dos_filbuf fb;
    if (_dos_files(&fb, path, 0x30) >= 0) {
        if (fb.atr & 0x10) {
            return MP_IMPORT_STAT_DIR;
        } else if (fb.atr & 0x20) {
            return MP_IMPORT_STAT_FILE;
        }
    }
    return MP_IMPORT_STAT_NO_EXIST;
}

STATIC mp_obj_t vfs_human_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);

    mp_obj_vfs_human_t *vfs = mp_obj_malloc(mp_obj_vfs_human_t, type);
    vstr_init(&vfs->root, 0);
    if (n_args == 1) {
        vstr_add_str(&vfs->root, mp_obj_str_get_str(args[0]));
        vstr_add_char(&vfs->root, '/');
    }
    vfs->root_len = vfs->root.len;
    vfs->readonly = false;

    return MP_OBJ_FROM_PTR(vfs);
}

STATIC mp_obj_t vfs_human_mount(mp_obj_t self_in, mp_obj_t readonly, mp_obj_t mkfs) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    if (mp_obj_is_true(readonly)) {
        self->readonly = true;
    }
    if (mp_obj_is_true(mkfs)) {
        mp_raise_OSError(MP_EPERM);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(vfs_human_mount_obj, vfs_human_mount);

STATIC mp_obj_t vfs_human_umount(mp_obj_t self_in) {
    (void)self_in;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(vfs_human_umount_obj, vfs_human_umount);

STATIC mp_obj_t vfs_human_open(mp_obj_t self_in, mp_obj_t path_in, mp_obj_t mode_in) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    const char *mode = mp_obj_str_get_str(mode_in);
    if (self->readonly
        && (strchr(mode, 'w') != NULL || strchr(mode, 'a') != NULL || strchr(mode, '+') != NULL)) {
        mp_raise_OSError(MP_EROFS);
    }
    if (!mp_obj_is_small_int(path_in)) {
        path_in = vfs_human_get_path_obj(self, path_in);
    }
    return mp_vfs_human_file_open(&mp_type_textio, path_in, mode_in);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(vfs_human_open_obj, vfs_human_open);

STATIC mp_obj_t vfs_human_chdir(mp_obj_t self_in, mp_obj_t path_in) {
    return vfs_human_fun1_helper(self_in, path_in, _dos_chdir);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(vfs_human_chdir_obj, vfs_human_chdir);

STATIC mp_obj_t vfs_human_getcwd(mp_obj_t self_in) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
//    char buf[MICROPY_ALLOC_PATH_MAX + 1];
//    const char *ret = getcwd(buf, sizeof(buf));     // TBD 
    const char *ret = NULL;
    if (ret == NULL) {
        mp_raise_OSError(errno);
    }
    ret += self->root_len;
    return mp_obj_new_str(ret, strlen(ret));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(vfs_human_getcwd_obj, vfs_human_getcwd);

typedef struct _vfs_human_ilistdir_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    bool is_str;
    bool active;
    struct dos_filbuf fb;
} vfs_human_ilistdir_it_t;

STATIC mp_obj_t vfs_human_ilistdir_it_iternext(mp_obj_t self_in) {
    vfs_human_ilistdir_it_t *self = MP_OBJ_TO_PTR(self_in);

    for (;;) {
        if (!self->active) {
            MP_THREAD_GIL_EXIT();
            int res = _dos_nfiles(&self->fb);
            if (res < 0) {
                MP_THREAD_GIL_ENTER();
                self->active = false;
                return MP_OBJ_STOP_ITERATION;
            }
            self->active = true;
            MP_THREAD_GIL_ENTER();
        }
        const char *fn = self->fb.name;

        if (fn[0] == '.' && (fn[1] == 0 || fn[1] == '.')) {
            // skip . and ..
            self->active = false;
            continue;
        }

        // make 3-tuple with info about this entry
        mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(3, NULL));

        if (self->is_str) {
            t->items[0] = mp_obj_new_str(fn, strlen(fn));
        } else {
            t->items[0] = mp_obj_new_bytes((const byte *)fn, strlen(fn));
        }

        if (self->fb.atr & 0x10) {
            t->items[1] = MP_OBJ_NEW_SMALL_INT(MP_S_IFDIR);
        } else if (self->fb.atr & 0x20) {
            t->items[1] = MP_OBJ_NEW_SMALL_INT(MP_S_IFREG);
        } else {
            t->items[1] = MP_OBJ_NEW_SMALL_INT(self->fb.atr);
        }

        t->items[2] = MP_OBJ_NEW_SMALL_INT(self->fb.filelen);

        self->active = false;
        return MP_OBJ_FROM_PTR(t);
    }
}

STATIC mp_obj_t vfs_human_ilistdir(mp_obj_t self_in, mp_obj_t path_in) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    vfs_human_ilistdir_it_t *iter = mp_obj_malloc(vfs_human_ilistdir_it_t, &mp_type_polymorph_iter);
    iter->iternext = vfs_human_ilistdir_it_iternext;
    iter->is_str = mp_obj_get_type(path_in) == &mp_type_str;
    const char *path = vfs_human_get_path_str(self, path_in);
    if (path[0] == '\0') {
        path = "*.*";
    }
    int res;
    MP_THREAD_GIL_EXIT();
    res = _dos_files(&iter->fb, path, 0x37);
    iter->active = true;
    MP_THREAD_GIL_ENTER();
    if (res < 0) {
        mp_raise_OSError(errno);
    }
    return MP_OBJ_FROM_PTR(iter);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(vfs_human_ilistdir_obj, vfs_human_ilistdir);

#if 0
typedef struct _mp_obj_listdir_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    DIR *dir;
} mp_obj_listdir_t;
#endif

STATIC mp_obj_t vfs_human_mkdir(mp_obj_t self_in, mp_obj_t path_in) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    const char *path = vfs_human_get_path_str(self, path_in);
    MP_THREAD_GIL_EXIT();
    int ret = _dos_mkdir(path);
    MP_THREAD_GIL_ENTER();
    if (ret < 0) {
        mp_raise_OSError(errno);        // TBD
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(vfs_human_mkdir_obj, vfs_human_mkdir);

STATIC mp_obj_t vfs_human_remove(mp_obj_t self_in, mp_obj_t path_in) {
    return vfs_human_fun1_helper(self_in, path_in, _dos_delete);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(vfs_human_remove_obj, vfs_human_remove);

STATIC mp_obj_t vfs_human_rename(mp_obj_t self_in, mp_obj_t old_path_in, mp_obj_t new_path_in) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    const char *old_path = vfs_human_get_path_str(self, old_path_in);
    const char *new_path = vfs_human_get_path_str(self, new_path_in);
    MP_THREAD_GIL_EXIT();
    int ret = _dos_rename(old_path, new_path);
    MP_THREAD_GIL_ENTER();
    if (ret < 0) {
        mp_raise_OSError(errno);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(vfs_human_rename_obj, vfs_human_rename);

STATIC mp_obj_t vfs_human_rmdir(mp_obj_t self_in, mp_obj_t path_in) {
    return vfs_human_fun1_helper(self_in, path_in, _dos_rmdir);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(vfs_human_rmdir_obj, vfs_human_rmdir);

STATIC mp_obj_t vfs_human_stat(mp_obj_t self_in, mp_obj_t path_in) {
#if 0
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    struct stat sb;
    const char *path = vfs_human_get_path_str(self, path_in);
    int ret;
    MP_HAL_RETRY_SYSCALL(ret, stat(path, &sb), mp_raise_OSError(err));
#endif
    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(10, NULL));
#if 0
    t->items[0] = MP_OBJ_NEW_SMALL_INT(sb.st_mode);
    t->items[1] = mp_obj_new_int_from_uint(sb.st_ino);
    t->items[2] = mp_obj_new_int_from_uint(sb.st_dev);
    t->items[3] = mp_obj_new_int_from_uint(sb.st_nlink);
    t->items[4] = mp_obj_new_int_from_uint(sb.st_uid);
    t->items[5] = mp_obj_new_int_from_uint(sb.st_gid);
    t->items[6] = mp_obj_new_int_from_uint(sb.st_size);
    t->items[7] = mp_obj_new_int_from_uint(sb.st_atime);
    t->items[8] = mp_obj_new_int_from_uint(sb.st_mtime);
    t->items[9] = mp_obj_new_int_from_uint(sb.st_ctime);
#endif
    return MP_OBJ_FROM_PTR(t);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(vfs_human_stat_obj, vfs_human_stat);

#if MICROPY_PY_UOS_STATVFS

#ifdef __ANDROID__
#define USE_STATFS 1
#endif

#if USE_STATFS
#include <sys/vfs.h>
#define STRUCT_STATVFS struct statfs
#define STATVFS statfs
#define F_FAVAIL sb.f_ffree
#define F_NAMEMAX sb.f_namelen
#define F_FLAG sb.f_flags
#else
#include <sys/statvfs.h>
#define STRUCT_STATVFS struct statvfs
#define STATVFS statvfs
#define F_FAVAIL sb.f_favail
#define F_NAMEMAX sb.f_namemax
#define F_FLAG sb.f_flag
#endif

STATIC mp_obj_t vfs_human_statvfs(mp_obj_t self_in, mp_obj_t path_in) {
    mp_obj_vfs_human_t *self = MP_OBJ_TO_PTR(self_in);
    STRUCT_STATVFS sb;
    const char *path = vfs_human_get_path_str(self, path_in);
    int ret;
    MP_HAL_RETRY_SYSCALL(ret, STATVFS(path, &sb), mp_raise_OSError(err));
    mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(10, NULL));
    t->items[0] = MP_OBJ_NEW_SMALL_INT(sb.f_bsize);
    t->items[1] = MP_OBJ_NEW_SMALL_INT(sb.f_frsize);
    t->items[2] = MP_OBJ_NEW_SMALL_INT(sb.f_blocks);
    t->items[3] = MP_OBJ_NEW_SMALL_INT(sb.f_bfree);
    t->items[4] = MP_OBJ_NEW_SMALL_INT(sb.f_bavail);
    t->items[5] = MP_OBJ_NEW_SMALL_INT(sb.f_files);
    t->items[6] = MP_OBJ_NEW_SMALL_INT(sb.f_ffree);
    t->items[7] = MP_OBJ_NEW_SMALL_INT(F_FAVAIL);
    t->items[8] = MP_OBJ_NEW_SMALL_INT(F_FLAG);
    t->items[9] = MP_OBJ_NEW_SMALL_INT(F_NAMEMAX);
    return MP_OBJ_FROM_PTR(t);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(vfs_human_statvfs_obj, vfs_human_statvfs);

#endif

STATIC const mp_rom_map_elem_t vfs_human_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_mount), MP_ROM_PTR(&vfs_human_mount_obj) },
    { MP_ROM_QSTR(MP_QSTR_umount), MP_ROM_PTR(&vfs_human_umount_obj) },
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&vfs_human_open_obj) },

    { MP_ROM_QSTR(MP_QSTR_chdir), MP_ROM_PTR(&vfs_human_chdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_getcwd), MP_ROM_PTR(&vfs_human_getcwd_obj) },
    { MP_ROM_QSTR(MP_QSTR_ilistdir), MP_ROM_PTR(&vfs_human_ilistdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_mkdir), MP_ROM_PTR(&vfs_human_mkdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_remove), MP_ROM_PTR(&vfs_human_remove_obj) },
    { MP_ROM_QSTR(MP_QSTR_rename), MP_ROM_PTR(&vfs_human_rename_obj) },
    { MP_ROM_QSTR(MP_QSTR_rmdir), MP_ROM_PTR(&vfs_human_rmdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_stat), MP_ROM_PTR(&vfs_human_stat_obj) },
    #if MICROPY_PY_UOS_STATVFS
    { MP_ROM_QSTR(MP_QSTR_statvfs), MP_ROM_PTR(&vfs_human_statvfs_obj) },
    #endif
};
STATIC MP_DEFINE_CONST_DICT(vfs_human_locals_dict, vfs_human_locals_dict_table);

STATIC const mp_vfs_proto_t vfs_human_proto = {
    .import_stat = mp_vfs_human_import_stat,
};

const mp_obj_type_t mp_type_vfs_human = {
    { &mp_type_type },
    .name = MP_QSTR_Vfshuman,
    .make_new = vfs_human_make_new,
    .protocol = &vfs_human_proto,
    .locals_dict = (mp_obj_dict_t *)&vfs_human_locals_dict,
};
