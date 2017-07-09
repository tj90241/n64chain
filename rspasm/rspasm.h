//
// rspasm/rspasm.h: RSP assembler common definitions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSPASM_RSPASM_H
#define RSPASM_RSPASM_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Forward declarations.
struct rspasm_identifiers;
struct rspasm_symbol;

struct rspasm {
  struct rspasm_identifiers *identifiers;
  struct rspasm_symbol *symbols;
  size_t num_symbols, max_symbols;

  // Binary data.
  size_t ihead;
  size_t dhead;
  uint8_t data[0x2000];

  // State.
  bool first_pass;
  bool in_text;
};

#endif

