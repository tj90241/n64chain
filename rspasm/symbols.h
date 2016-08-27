//
// rspasm/symbols.h: RSP assmebler symbol functionality.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSPASM_SYMBOLS_H
#define RSPASM_SYMBOLS_H
#include "rspasm.h"

int rspasm_add_symbol(struct rspasm *rspasm,
  const char *name, uint32_t addr);

int rspasm_do_symbols_pass(struct rspasm *rspasm);

int rspasm_get_symbol_address(const struct rspasm *rspasm,
  const char *name, uint32_t *addr);

void rspasm_free_symbols(const struct rspasm *rspasm);

#endif

