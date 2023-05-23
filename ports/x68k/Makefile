include ../../py/mkenv.mk

CROSS ?= 1

include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

ifeq ($(CROSS), 1)
CROSS_COMPILE ?= m68k-xelf-
endif

export LC_MESSAGES=C

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

LD = $(CC)
CFLAGS += $(INC) -Os
CFLAGS += -Wall -Werror
LDFLAGS += -Wl,-Map=$@.map,--cref -Wl,--gc-sections
#CFLAGS += -DNDEBUG

SRC_C = \
	main.c \
	mphalport.c \
	vfs_human.c \
	vfs_human_file.c \
	input.c \
	modutime.c \
	modmachine.c \
	machine_pin.c \
	modx68k.c \
	modx68kgvram.c \
	modx68ktvram.c \
	modx68kspr.c \
	modx68kiocs.c \
	modx68kdos.c \
	modx68kint.c \
	modx68kxarray.c \
	shared/readline/readline.c \
	shared/runtime/gchelper_generic.c \
	shared/runtime/pyexec.c \
	shared/runtime/stdout_helpers.c \
	$(SRC_MOD) \
	$(SHARED_SRC_C_EXTRA)

SRC_CXX += \
	$(SRC_MOD_CXX)

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_CXX) $(SHARED_SRC_C) $(EXTMOD_SRC_C)
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS +=

TARGET := $(BUILD)/micropython
TARGETELF = $(TARGET)
ifeq ($(CROSS), 1)
TARGET := $(TARGET).x
TARGETELF = $(TARGET).elf
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ -lm
	$(Q)$(SIZE) $(TARGETELF)

include $(TOP)/py/mkrules.mk

mpyconv-all:
	make -C mpyconv all

mpyconv-clean:
	make -C mpyconv clean

release: clean all mpyconv-clean mpyconv-all
	cp README*.md build
	rm -rf build/sample
	mkdir build/sample
	for f in sample/*.py; do \
		sed 's/$$/\r/' $$f > build/$$f; \
	done
	cp mpyconv/mpyconv.x build
	(cd build; zip -r micropython-$(shell git describe).zip micropython.x mpyconv.x *.md sample)
