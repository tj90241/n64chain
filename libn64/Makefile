#
# libn64/Makefile: Makefile for libn64.
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

AS = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-as)
AR = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc-ar)
CC = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc)

CFLAGS = -Wall -Wextra -pedantic -std=c99 -I. -Iinclude -Ipriv_include
OPTFLAGS = -Os -march=vr4300 -mtune=vr4300 -mabi=eabi -mgp32 -mlong32 \
	-flto -ffat-lto-objects -ffunction-sections -fdata-sections \
	-G4 -mno-extern-sdata -mgpopt -mfix4300 -mbranch-likely

ASMFILES = $(call FIXPATH,\
	os/asm/boot.s \
	os/asm/context.s \
	os/asm/exception.s \
	os/asm/idle_thread.s \
	os/asm/message.s \
	os/asm/syscall.s \
)

CFILES = $(call FIXPATH,\
	os/fbtext.c \
	os/main.c \
	os/mm.c \
	os/panic.c \
	os/thread.c \
	os/time.c \
	rcp/vi.c \
)

OBJFILES = \
	$(CFILES:.c=.o) \
	$(ASMFILES:.s=.o)

DEPFILES = $(OBJFILES:.o=.d)

#
# Primary targets.
#
libn64.a: $(OBJFILES)
	@echo $(call FIXPATH,"Building: libn64/$@")
	@$(AR) rcs $@ $^

#
# Generic compilation/assembly targets.
#
%.o: %.s
	@echo $(call FIXPATH,"Assembling: libn64/$<")
	@$(CC) -x assembler-with-cpp $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

%.o: %.c include/syscall.h
	@echo $(call FIXPATH,"Compiling: libn64/$<")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

#
# Clean project target.
#
.PHONY: clean
clean:
	@echo "Cleaning libn64..."
	@$(RM) libn64.a $(DEPFILES) $(OBJFILES)

