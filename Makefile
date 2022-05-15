# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2020 StarFiveTech, Inc.

EMPTY  :=
SPACE  := $(EMPTY) $(EMPTY)
PROGRAM = jh7100_recovery_boot

SUFFIX=$(shell date +%y%m%d)
GIT_VERSION=$(shell git show -s --pretty=format:%h)
VERSION=$(SUFFIX)-$(GIT_VERSION)

#############################################################
# Makefile Arguments
#############################################################

# The configuration defaults to Debug. Valid choices are:
#  - debug
#  - release
CONFIGURATION ?= debug
BSP_DIR ?= bsp

#############################################################
# BSP loading
#############################################################

# There must be a settings makefile fragment in the BSP's board directory.
ifeq ($(wildcard $(BSP_DIR)/settings.mk),)
$(error Unable to find BSP for $(TARGET), expected to find $(BSP_DIR)/settings.mk)
endif

# Include the BSP settings
include $(BSP_DIR)/settings.mk

# Check that settings.mk sets RISCV_ARCH and RISCV_ABI
ifeq ($(RISCV_ARCH),)
$(error $(BSP_DIR)/settings.mk must set RISCV_ARCH, the RISC-V ISA string to target)
endif

ifeq ($(RISCV_ABI),)
$(error $(BSP_DIR)/settings.mk must set RISCV_ABI, the ABI to target)
endif

ifeq ($(RISCV_CMODEL),)
RISCV_CMODEL = medany
endif

ifeq ($(LINK_TARGET),)
LINK_TARGET = bootloader
endif

LINKER_SCRIPT = $(BSP_DIR)/$(LINK_TARGET).lds

# Determines the XLEN from the toolchain tuple
ifeq ($(patsubst rv32%,rv32,$(RISCV_ARCH)),rv32)
RISCV_XLEN := 32
else ifeq ($(patsubst rv64%,rv64,$(RISCV_ARCH)),rv64)
RISCV_XLEN := 64
else
$(error Unable to determine XLEN from $(RISCV_ARCH))
endif

#############################################################
# Toolchain
#############################################################

# Allow users to select a different cross compiler.
CROSS_COMPILE = riscv64-unknown-elf-

# If users don't specify RISCV_PATH then assume that the tools will just be in
# their path.
ifeq ($(RISCV_PATH),)
CC      := $(CROSS_COMPILE)gcc
CXX     := $(CROSS_COMPILE)g++
OBJDUMP := $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy
GDB     := $(CROSS_COMPILE)gdb
AR      := $(CROSS_COMPILE)ar
SIZE    := $(CROSS_COMPILE)size
else
CC      := $(abspath $(RISCV_PATH)/bin/$(CROSS_COMPILE)gcc)
CXX     := $(abspath $(RISCV_PATH)/bin/$(CROSS_COMPILE)g++)
OBJDUMP := $(abspath $(RISCV_PATH)/bin/$(CROSS_COMPILE)objdump)
OBJCOPY := $(abspath $(RISCV_PATH)/bin/$(CROSS_COMPILE)objcopy)
GDB     := $(abspath $(RISCV_PATH)/bin/$(CROSS_COMPILE)gdb)
AR      := $(abspath $(RISCV_PATH)/bin/$(CROSS_COMPILE)ar)
SIZE    := $(abspath $(RISCV_PATH)/bin/$(CROSS_COMPILE)size)
PATH    := $(abspath $(RISCV_PATH)/bin):$(PATH)
endif

#############################################################
# Software Flags
#############################################################

# Set the arch, ABI, and code model
CCASFLAGS += -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI) -mcmodel=$(RISCV_CMODEL) -DVERSION=\"$(VERSION)\" -DCONFIGURATION=\"$(CONFIGURATION)\"
CFLAGS    += -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI) -mcmodel=$(RISCV_CMODEL) -DVERSION=\"$(VERSION)\" -DCONFIGURATION=\"$(CONFIGURATION)\"
CXXFLAGS  += -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI) -mcmodel=$(RISCV_CMODEL) -DVERSION=\"$(VERSION)\" -DCONFIGURATION=\"$(CONFIGURATION)\"
ASFLAGS   += -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI) -mcmodel=$(RISCV_CMODEL) -DVERSION=\"$(VERSION)\" -DCONFIGURATION=\"$(CONFIGURATION)\"
# Prune unused functions and data
CFLAGS   += -ffunction-sections -fdata-sections
CXXFLAGS += -ffunction-sections -fdata-sections
# Use newlib-nano
CCASFLAGS += --specs=nano.specs
CFLAGS    += --specs=nano.specs
CXXFLAGS  += --specs=nano.specs
# DDR speed config for c code
CFLAGS += -DDDR_SPEED=$(DDR_SPEED)

# Set the arch, ABI, and code model
LDFLAGS += -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI) -mcmodel=$(RISCV_CMODEL)
# Turn on garbage collection for unused sections
LDFLAGS += -Wl,--gc-sections
# Turn off the C standard library
LDFLAGS += -nostartfiles -nostdlib
# Find the linker scripts
LDFLAGS += -T$(filter %.lds,$^)

# Link to the relevant libraries
LDLIBS += -Wl,--start-group -lc -lgcc -lm -Wl,--end-group

# Load the configuration Makefile
CONFIGURATION_FILE = $(wildcard $(CONFIGURATION).mk)
ifeq ($(words $(CONFIGURATION_FILE)),0)
$(error Unable to find the Makefile $(CONFIGURATION).mk for CONFIGURATION=$(CONFIGURATION))
endif
include $(CONFIGURATION).mk

#############################################################
# Software
#############################################################
OUT_DIR = $(CONFIGURATION)
OBJ_DIR = $(OUT_DIR)/objs
PROGRAM_ELF = $(OUT_DIR)/$(PROGRAM).elf
PROGRAM_BIN = $(OUT_DIR)/$(PROGRAM).bin
PROGRAM_LST = $(OUT_DIR)/$(PROGRAM).lst

#############################################################
# Makefile Functions
#############################################################

# fn_srcfiles(dirs,ext)
fn_srcfiles = $(foreach d,$(1),$(wildcard $(d)/*.$(2)))

# fn_depfiles(dirs,ext)
fn_depfiles = $(foreach f,$(call fn_srcfiles,$(1),$(2)),$(OBJ_DIR)/$(patsubst %.$(2),%.d,$(f)))

# fn_objfiles(dirs,ext)
fn_objfiles = $(foreach f,$(call fn_srcfiles,$(1),$(2)),$(OBJ_DIR)/$(patsubst %.$(2),%.o,$(f)))

# fn_includes(dirs)
fn_includes = $(addprefix -I,$(1))

#############################################################
# Include and Source Dirs
#############################################################
INCLUDE_DIRS += \
	src/boot \
	src/common \
	src/driver/uart \
	src/driver/spi \
	src/driver/gpio \
	src/driver/timer \

SOURCE_DIRS += \
	src/boot \
	src/common \
	src/driver/uart \
	src/driver/spi \
	src/driver/gpio \
	src/driver/timer \
	src/driver/xmodem \

INCLUDES = $(call fn_includes,$(INCLUDE_DIRS))

PROGRAM_SRCS = \
	$(call fn_srcfiles,$(SOURCE_DIRS),S) \
	$(call fn_srcfiles,$(SOURCE_DIRS),c) \

PROGRAM_OBJS = \
	$(call fn_objfiles,$(SOURCE_DIRS),S) \
	$(call fn_objfiles,$(SOURCE_DIRS),c) \

#VPATH := $(subst $(SPACE),:,$(SOURCE_DIRS) $(INCLUDE_DIRS))

#############################################################
# Makefile Targets
#############################################################
all: $(PROGRAM_ELF)


#############################################################
# Depenences
#############################################################
PROGRAM_DEPS = \
	$(call fn_depfiles,$(SOURCE_DIRS),S) \
	$(call fn_depfiles,$(SOURCE_DIRS),c) \

ifneq ($(strip $(PROGRAM_DEPS)),)
sinclude $(PROGRAM_DEPS)
endif

#############################################################
# Makefile Rules
#############################################################

$(PROGRAM_ELF): $(PROGRAM_OBJS) $(LINKER_SCRIPT)
	@echo Building target: $@, VERSION=$(VERSION)
	@echo Invoking: GNU RISC-V Cross C Linker
	@if [ ! -d "$(@D)" ]; then mkdir -p "$(@D)"; fi
	$(CC) $(LDFLAGS) -Wl,-Map,"$(@:%.elf=%.map)" $(filter %.o,$^) $(LDLIBS) -o $@
	@echo Finished building target: $@
	@$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $@ > $(PROGRAM_LST)
	@$(OBJCOPY) -O binary $@ $(PROGRAM_BIN)

$(OBJ_DIR)/%.o: %.c
	@if [ ! -d "$(@D)" ]; then mkdir -p "$(@D)"; fi
	@echo cc $<
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<

$(OBJ_DIR)/%.o: %.S
	@if [ ! -d "$(@D)" ]; then mkdir -p "$(@D)"; fi
	@echo cc $<
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o $@ $<

.PHONY: debug
debug:
	@echo ----- INCLUDES:
	@echo $(INCLUDES)
	@echo ----- PROGRAM_SRCS:
	@echo $(PROGRAM_SRCS)
	@echo ----- PROGRAM_ELF:
	@echo $(PROGRAM_ELF)
	@echo ----- PROGRAM_DEPS:
	@echo $(PROGRAM_DEPS)
	@echo ----- PROGRAM_OBJS:
	@echo $(PROGRAM_OBJS)

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)

