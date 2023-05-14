include ../../../py/mkenv.mk

CROSS ?= 1

ifeq ($(CROSS), 1)
CROSS_COMPILE ?= m68k-xelf-
endif

export LC_MESSAGES=C

# qstr definitions (must come before including py.mk)
QSTR_DEFS = $(TOP)/mpy-cross/qstrdefsport.h

# include py core make definitions
include $(TOP)/py/py.mk

INC += -I.
INC += -I$(TOP)/mpy-cross
INC += -I$(BUILD)
INC += -I$(TOP)

# compiler settings
CWARN = -Wall -Werror
CWARN += -Wextra -Wno-unused-parameter -Wpointer-arith
CFLAGS = $(INC) $(CWARN) -std=gnu99 $(COPT) $(CFLAGS_EXTRA)
CFLAGS += -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables

# Debugging/Optimization
CFLAGS += -g
COPT = -Os

# Use gcc syntax for map file
LDFLAGS_ARCH = -Wl,-Map=$@.map,--cref -Wl,--gc-sections
LDFLAGS = $(LDFLAGS_MOD) $(LDFLAGS_ARCH) -lm $(LDFLAGS_EXTRA)

# source files
SRC_C = \
	mpy-cross/main.c \
	mpy-cross/gccollect.c \
	shared/runtime/gchelper_generic.c \

OBJ = $(PY_CORE_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

TARGET := mpyconv.x
TARGETELF = $(TARGET).elf

all: $(TARGET)

$(TARGET): $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) -o $@ $^ $(LIB) $(LDFLAGS)
	$(Q)$(SIZE) $(TARGETELF)

clean: clean-prog
clean-prog:
	$(RM) -f $(TARGET) $(TARGETELF) $(TARGETELF).2
	$(RM) -f $(TARGET).map

.PHONY: clean-prog

include $(TOP)/py/mkrules.mk
