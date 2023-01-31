include ../../py/mkenv.mk

CROSS ?= 1

include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

ifeq ($(CROSS), 1)
CROSS_COMPILE ?= m68k-elf-
endif

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

LD = $(CC)
CFLAGS += $(INC) -Os
LDFLAGS += -Wl,-Map=$@.map,--cref -Wl,--gc-sections
#CFLAGS += -DNDEBUG

SRC_C = \
	main.c \
	mphalport.c \
	shared/readline/readline.c \
	shared/runtime/gchelper_generic.c \
	shared/runtime/pyexec.c \
	shared/runtime/stdout_helpers.c

OBJ += $(PY_CORE_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

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
