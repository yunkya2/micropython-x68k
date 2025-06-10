include ../../py/mkenv.mk

export LC_MESSAGES=C

CROSS ?= 1

# Use the default frozen manifest, variants may override this.
FROZEN_MANIFEST ?= manifest.py

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

ifeq ($(CROSS), 1)
CROSS_COMPILE ?= m68k-xelf-
endif

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

LD = $(CC)
CFLAGS += $(INC) -Wall -Werror
LDFLAGS += -Wl,-Map=$@.map,--cref -Wl,--gc-sections

# Tune for Debugging or Optimization
CFLAGS += -g  # always include debug info in the ELF
ifeq ($(DEBUG), 1)
CFLAGS += -O0
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

SRC_C = \
	main.c \
	gccollect.c \
	mphalport.c \
	vfs_human.c \
	vfs_human_file.c \
	input.c \
	machine_pin.c \
	modx68k.c \
	modx68kgvram.c \
	modx68ktvram.c \
	modx68kspr.c \
	modx68kiocs.c \
	modx68kdos.c \
	modx68kint.c \
	modx68kfnc.c \
	modx68kxarray.c \
	$(SRC_MOD) \
	$(SHARED_SRC_C_EXTRA)

SHARED_SRC_C += $(addprefix shared/,\
	readline/readline.c \
	runtime/gchelper_generic.c \
	runtime/stdout_helpers.c \
	$(SHARED_SRC_C_EXTRA) \
	)

SRC_CXX += \
	$(SRC_MOD_CXX)

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(EXTMOD_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_CXX) $(SHARED_SRC_C)

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

RELFILE := micropython-$(shell git describe)

release: clean all mpyconv-clean mpyconv-all
	cp README*.md build
	./md2txtconv.py -r build/*.md
	rm -rf build/sample
	mkdir build/sample
	for f in sample/*.py; do \
		sed 's/$$/\r/' $$f > build/$$f; \
	done
	cp mpyconv/mpyconv.x build
	(cd build; zip -r $(RELFILE).zip micropython.x mpyconv.x *.txt sample)
	(cd build; ../xdftool/xdftool.py c $(RELFILE).xdf micropython.x mpyconv.x *.txt sample)
	(cd build; zip -r $(RELFILE).xdf.zip $(RELFILE).xdf)
