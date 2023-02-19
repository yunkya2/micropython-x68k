// m68k specific stuff

#include "py/mpconfig.h"

#if MICROPY_EMIT_M68K

// this is defined so that the assembler exports generic assembler API macros
#define GENERIC_ASM_API (1)
#include "py/asmm68k.h"

#define N_M68K (1)
#define EXPORT_FUN(name) emit_native_m68k_##name
#include "py/emitnative.c"

#endif
