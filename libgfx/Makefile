#
# libgfx/Makefile: Makefile for libgfx.
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
CPP = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-cpp)

RSPASM = $(call FIXPATH,$(CURDIR)/../tools/bin/rspasm)

CFLAGS = -Wall -Wextra -pedantic -std=c99 -I. -Iinclude -I../libn64/include
OPTFLAGS = -Os -march=vr4300 -mtune=vr4300 -mabi=eabi -mgp32 -mlong32 \
	-flto -ffat-lto-objects -ffunction-sections -fdata-sections \
	-G4 -mno-extern-sdata -mgpopt -mfix4300 -mbranch-likely \
	-mno-check-zero-division -mno-memcpy

ASMFILES = $(call FIXPATH,\
)

CFILES = $(call FIXPATH,\
	src/init.c \
	src/rspbuf.c \
)

UCODES = $(call FIXPATH,\
	ucodes/gfx.rsp \
)

OBJFILES = \
	$(CFILES:.c=.o) \
	$(ASMFILES:.s=.o) \
	$(UCODES:.rsp=.o)

DEPFILES = $(OBJFILES:.o=.d)

#
# Primary targets.
#
libgfx.a: $(OBJFILES)
	@echo $(call FIXPATH,"Building: libgfx/$@")
	@$(AR) rcs $@ $^

#
# Generic compilation/assembly targets.
#
%.o: %.s
	@echo $(call FIXPATH,"Assembling: libgfx/$<")
	@$(CC) -x assembler-with-cpp $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

%.o: %.c
	@echo $(call FIXPATH,"Compiling: libgfx/$<")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

%.o: %.rsp %.rsps
	@echo $(call FIXPATH,"Assembling: $(ROM_NAME)/$@")
	@$(CPP) -E -I../libn64/ucodes -Iucodes $< > $(<:.rsp=.rsppch)
	@$(RSPASM) $(<:.rsp=.rsppch) -o $(<:.rsp=.bin)
	@$(CC) -x assembler-with-cpp $(CFLAGS) $(OPTFLAGS) -MMD -c $(<:.rsp=.rsps) -o $@

#
# Clean project target.
#
.PHONY: clean
clean:
	@echo "Cleaning libgfx..."
	@$(RM) libgfx.a $(DEPFILES) $(OBJFILES)

