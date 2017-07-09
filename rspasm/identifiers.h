//
// rspasm/identifiers.h: RSP assmebler identifiers functionality.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSPASM_IDENTIFIERS_H
#define RSPASM_IDENTIFIERS_H
#include "rspasm.h"

enum rspasm_identifier_node_color {
  RSPASM_IDENTIFIER_NODE_BLACK,
  RSPASM_IDENTIFIER_NODE_RED
};

enum rspasm_identifier_node_type {
  RSPASM_IDENTIFIER_NODE_INT,
  RSPASM_IDENTIFIER_NODE_REG,
  RSPASM_IDENTIFIER_NODE_VREG,
};

struct rspasm_identifier_node {
  struct rspasm_identifier_node *left;
  struct rspasm_identifier_node *parent;
  struct rspasm_identifier_node *right;

  enum rspasm_identifier_node_color color;
  enum rspasm_identifier_node_type type;

  char *name;
  int32_t value;
};

struct rspasm_identifiers {
  struct rspasm_identifier_node *nil;
  struct rspasm_identifier_node *root;
};

int rspasm_identifiers_create(struct rspasm_identifiers *ids);
void rspasm_identifiers_destroy(struct rspasm_identifiers *ids);

int rspasm_identifiers_get(const struct rspasm_identifiers *ids,
    const char *name, int32_t *value, enum rspasm_identifier_node_type *type);

int rspasm_identifiers_set(struct rspasm_identifiers *ids,
    const char *name, int32_t value, enum rspasm_identifier_node_type type);

int rspasm_identifiers_unset(struct rspasm_identifiers *ids,
    const char *name);

#endif

