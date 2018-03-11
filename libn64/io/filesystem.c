//
// libn64/io/filesystem.c: Filesystem operations.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <io.h>
#include <io/filesystem.h>
#include <libn64.h>
#include <stdint.h>

// Issues a request to the PI subsystem to load a file.
void filesystem_load(const struct libn64_pi_request *pi_req) {
  uint32_t pi_addr;

  extern char _binary_filesystem_bin_start;
  static const char *fs_ptr = &_binary_filesystem_bin_start;
  uint32_t fs_offs;

  // PI transfer lengths must be an even number of bytes.
  // The source address must also be 2-byte aligned, but the
  // filesystem packer takes care of doing that for us.
  uint32_t transfer_size = (pi_req->size + 1) & ~0x1;

  // Cart domain is 0x1000_0000, a 0x1000 header is stapled onto
  // the cart by checksum, and the cart gets loaded to 0x8000_0400.
  // Do a bit of fudging to figure out where the data is located.
  __builtin_memcpy(&fs_offs, &fs_ptr, sizeof(fs_offs));
  fs_offs -= 0x70000400;

  __asm__ __volatile__(
    ".set noreorder\n\t"

    "lui %0, 0xA460\n\t"
    "sw %1, 0x0(%0)\n\t"
    "sw %2, 0x4(%0)\n\t"
    "sw %3, 0xC(%0)\n\t"

    ".set reorder\n\t"

    : "=&r" (pi_addr)
    : "r" (pi_req->dest_address),
      "r" (pi_req->src_address + fs_offs),
      "r" (transfer_size)
  );
}

