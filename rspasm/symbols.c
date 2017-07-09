//
// rspasm/symbols.c: RSP assmebler symbol functionality.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "rspasm.h"
#include "symbols.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RSPASM_DEFAULT_NUM_SYMBOLS 128

struct rspasm_symbol {
  char *name;
  int32_t addr;
};

static int rspasm_symbol_binary_search(const struct rspasm_symbol *symbols,
  const char *name, long long mini, long long maxi);

static int rspasm_symbols_compare(const void *a, const void *b);

int rspasm_add_symbol(struct rspasm *rspasm,
  const char *name, uint32_t addr) {
  struct rspasm_symbol *symbol;

  size_t name_len;
  char *lname;

  // Allocate memory for another symbol.
  if (rspasm->num_symbols >= rspasm->max_symbols) {
    struct rspasm_symbol *symbols;

    size_t num_symbols = rspasm->num_symbols == 0
      ? RSPASM_DEFAULT_NUM_SYMBOLS
      : rspasm->num_symbols * 2;

    if ((symbols = malloc(sizeof(*symbols) * num_symbols)) == NULL) {
      fprintf(stderr, "Out of memory.\n");
      return -1;
    }

    memcpy(symbols, rspasm->symbols,
      sizeof(*symbols) * rspasm->max_symbols);

    free(rspasm->symbols);
    rspasm->symbols = symbols;
    rspasm->max_symbols = num_symbols;
  }

  // Allocate memory for the name, copy it.
  name_len = strlen(name);

  if ((lname = malloc(name_len + 1)) == NULL) {
    fprintf(stderr, "Out of memory.\n");
    return -1;
  }

  memcpy(lname, name, name_len);
  lname[name_len] = '\0';

  // Store the symbol data.
  symbol = rspasm->symbols + (rspasm->num_symbols++);

  symbol->name = lname;
  symbol->addr = addr;
  return 0;
}

int rspasm_do_symbols_pass(struct rspasm *rspasm) {
  size_t i;

  qsort(rspasm->symbols, rspasm->num_symbols,
    sizeof(*rspasm->symbols), rspasm_symbols_compare);

  for (i = 1; i < rspasm->num_symbols; i++) {
    if (!strcmp(rspasm->symbols[i].name, rspasm->symbols[i - 1].name)) {
      fprintf(stderr, "Symbol defined twice: %s\n", rspasm->symbols[i].name);
      return -1;
    }
  }

  return 0;
}

void rspasm_free_symbols(const struct rspasm *rspasm) {
  if (rspasm->symbols != NULL) {
    size_t i;

    for (i = 0; i < rspasm->num_symbols; i++)
      free(rspasm->symbols[i].name);
  }

  free(rspasm->symbols);
}

int rspasm_get_symbol_address(const struct rspasm *rspasm,
  const char *name, uint32_t *addr) {
  int chk_addr;

  if (rspasm->num_symbols == 0)
    return -1;

  if ((chk_addr = rspasm_symbol_binary_search(
    rspasm->symbols, name, 0, rspasm->num_symbols - 1)) < 0) {
    return -1;
  }

  *addr = (uint32_t) chk_addr;
  return 0;
}

int rspasm_symbol_binary_search(const struct rspasm_symbol *symbols,
  const char *name, long long mini, long long maxi) {
  size_t midi;
  int cmp;

  if (maxi < mini)
    return -1;

  midi = (mini + maxi) / 2;
  cmp = strcmp(symbols[midi].name, name);

  if (cmp > 0)
    return rspasm_symbol_binary_search(symbols, name, mini, midi - 1);

  else if (cmp < 0)
    return rspasm_symbol_binary_search(symbols, name, midi + 1, maxi);

  return (int) symbols[midi].addr;
}

int rspasm_symbols_compare(const void *a, const void *b) {
  const struct rspasm_symbol *sa = (const struct rspasm_symbol *) a;
  const struct rspasm_symbol *sb = (const struct rspasm_symbol *) b;

  return strcmp(sa->name, sb->name);
}

