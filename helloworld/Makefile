#
# Makefile for n64chain ROMs.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

ifdef SystemRoot
FIXPATH = $(subst /,\,$1)
RM = del /Q
else
FIXPATH = $1
RM = rm -f
endif

ROM_NAME = $(notdir $(CURDIR))

AS = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-as)
AR = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc-ar)
CC = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc)
CPP = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-cpp)
MAKE = $(call FIXPATH,$(CURDIR)/../tools/bin/make)
MKFS = $(call FIXPATH,$(CURDIR)/../tools/bin/mkfs)
OBJCOPY = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-objcopy)

CHECKSUM = $(call FIXPATH,$(CURDIR)/../tools/bin/checksum)
RSPASM = $(call FIXPATH,$(CURDIR)/../tools/bin/rspasm)

# priv_include is not "advertised", but this is just a demo...
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Wno-main \
	-I../libn64/include -I../libn64 -I../libn64/priv_include -I.

OPTFLAGS = -Os -march=vr4300 -mtune=vr4300 -mabi=eabi -mgp32 -mlong32 \
	-flto -ffat-lto-objects -ffunction-sections -fdata-sections \
	-G4 -mno-extern-sdata -mgpopt -mfix4300 -mbranch-likely \
	-mno-check-zero-division

ASMFILES = $(call FIXPATH,\
)

CFILES = $(call FIXPATH,\
	src/main.c \
)

UCODES = $(call FIXPATH,\
	$(wildcard ucodes/*.rsp) \
)

OBJFILES = \
	$(ASMFILES:.S=.o) \
	$(CFILES:.c=.o)

UCODETSKS = $(foreach ucode, $(UCODES),\
	filesystem/$(basename $(notdir $(ucode))).tsk)

DEPFILES = $(OBJFILES:.o=.d)

#
# Primary targets.
#
all: $(ROM_NAME).z64

$(ROM_NAME).z64: $(ROM_NAME).elf
	@echo $(call FIXPATH,"Building: $(ROM_NAME)/$@")
	@$(OBJCOPY) -O binary $< $@
	@$(CHECKSUM) $(call FIXPATH,../libn64/header.bin) $@

$(ROM_NAME).elf: libn64 $(OBJFILES) filesystem.obj
	@echo $(call FIXPATH,"Building: $(ROM_NAME)/$@")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -Wl,-Map=$(ROM_NAME).map -nostdlib \
		-T$(call FIXPATH,../libn64/rom.ld) -o $@ $(OBJFILES) filesystem.obj \
		-L$(call FIXPATH,../libn64) -ln64

#
# Filesystem build target.
#
filesystem.obj: filesystem.h
	@echo $(call FIXPATH,"Building: $(ROM_NAME)/$@")
	@$(OBJCOPY) -I binary -O elf32-bigmips -B mips filesystem.bin $@

filesystem.h: $(wildcard filesystem/*) $(UCODETSKS)
	@echo $(call FIXPATH,"Generate: $(ROM_NAME)/$@")
	@$(MKFS) filesystem.bin filesystem.h filesystem

#
# Generic compilation/assembly targets.
#
%.o: %.S filesystem.h
	@echo $(call FIXPATH,"Assembling: $(ROM_NAME)/$<")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

%.o: %.c filesystem.h
	@echo $(call FIXPATH,"Compiling: $(ROM_NAME)/$<")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

filesystem/%.tsk: ucodes/%.rsp
	@echo $(call FIXPATH,"Assembling: $(ROM_NAME)/$@")
	@$(CPP) -E -I../libn64/ucodes -Iucodes $< | $(RSPASM) -o $@ -

.PHONY: libn64
libn64:
	@$(MAKE) -sC $(call FIXPATH,../libn64)

#
# Clean project target.
#
.PHONY: clean
clean:
	@echo "Cleaning $(ROM_NAME)..."
	@$(RM) $(ROM_NAME).map $(ROM_NAME).elf $(ROM_NAME).z64 \
		$(DEPFILES) $(OBJFILES) $(UCODEBINS) filesystem.obj \
		filesystem.bin filesystem.h

#
# Use computed dependencies.
#
-include $(DEPFILES)

