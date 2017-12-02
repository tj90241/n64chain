//
// libn64/os/time.c: OS time functions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>

// Initialize the time subsystem.
void libn64_time_init(void) {

  // Bounce the timer so that it starts firing.
  // Initialize the global time counter to zero.
  __asm__ __volatile__(
    ".set noat\n\t"
    ".set gp=64\n\t"
    "addiu $at, $zero, -0x1\n\t"
    "mtc0 $at, $11\n\t"
    "mtc0 $zero, $9\n\t"
    "lui $at, 0x8000\n\t"
    "sd $zero, 0x448($at)\n\t"
    ".set gp=default\n\t"
    ".set at\n\t"
  );
}

