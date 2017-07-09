//
// rspasm/identifiers.c: RSP assmebler identifiers functionality.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "identifiers.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void delete_fixup(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *node);

static void fixup(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *node);

static void rotate_left(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *n);

static void rotate_right(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *n);

static void transplant(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *u, struct rspasm_identifier_node *v);

static struct rspasm_identifier_node *tree_minimum(
    struct rspasm_identifiers *ids, struct rspasm_identifier_node *node);

int rspasm_identifiers_create(struct rspasm_identifiers *ids) {
  if ((ids->nil = malloc(sizeof(*ids->nil))) == NULL) {
    return 1;
  }

  ids->nil->name = NULL;
  ids->root = ids->nil;
  return 0;
}

void rspasm_identifiers_destroy(struct rspasm_identifiers *ids) {
  while (ids->root != ids->nil)
    rspasm_identifiers_unset(ids, ids->root->name);

  free(ids->nil);
}

int rspasm_identifiers_get(const struct rspasm_identifiers *ids,
    const char *name, int32_t *value, enum rspasm_identifier_node_type *type) {
  const struct rspasm_identifier_node *cur = ids->root;

  while (cur != ids->nil) {
    int cmp = strcmp(name, cur->name);

    if (cmp < 0)
      cur = cur->left;

    else if (cmp > 0)
      cur = cur->right;

    else {
      *value = cur->value;
      *type = cur->type;
      return 1;
    }
  }

  return 0;
}

int rspasm_identifiers_set(struct rspasm_identifiers *ids,
    const char *name, int32_t value, enum rspasm_identifier_node_type type) {
  struct rspasm_identifier_node *check = ids->root;
  struct rspasm_identifier_node *cur = ids->nil;
  struct rspasm_identifier_node *new_node;
  char *name_copy;
  int cmp = cmp;

  // Find the current value/figure out where to insert ourself.
  while (check != ids->nil) {
    cur = check;

    if ((cmp = strcmp(name, cur->name)) != 0) {
      check = cmp < 0
        ? check->left
        : check->right;
    }

    else {
      check->value = value;
      check->type = type;
      return 0;
    }
  }

  // Not in the tree yet; add a new node.
  if ((new_node = malloc(sizeof(*new_node))) == NULL) {
    fprintf(stderr, "Out of memory.\n");
    return 1;
  }

  if ((name_copy = malloc(strlen(name) + 1)) == NULL) {
    fprintf(stderr, "Out of memory.\n");
    free(new_node);
    return 1;
  }

  strcpy(name_copy, name);

  if (cur == ids->nil)
    ids->root = new_node;

  else if (cmp < 0)
    cur->left = new_node;

  else
    cur->right = new_node;

  new_node->left = ids->nil;
  new_node->right = ids->nil;
  new_node->parent = cur;
  new_node->name = name_copy;
  new_node->type = type;
  new_node->value = value;

  /* Rebalance the tree as needed */
  new_node->color = RSPASM_IDENTIFIER_NODE_RED;
  fixup(ids, new_node);
  return 0;
}

int rspasm_identifiers_unset(struct rspasm_identifiers *ids, const char *name) {
  struct rspasm_identifier_node *cur = ids->root;

  while (cur != ids->nil) {
    int cmp = strcmp(name, cur->name);

    if (cmp < 0)
      cur = cur->left;

    else if (cmp > 0)
      cur = cur->right;

    else {
      struct rspasm_identifier_node *temp, *node = cur;
      enum rspasm_identifier_node_color node_orig_color = node->color;

      if (cur->left == ids->nil) {
        temp = cur->right;
        transplant(ids, cur, cur->right);
      }

      else if (cur->right == ids->nil) {
        temp = cur->left;
        transplant(ids, cur, cur->left);
      }

      else {
        node = tree_minimum(ids, cur->right);
        node_orig_color = node->color;
        temp = node->right;

        if (node->parent == cur)
          temp->parent = node;

        else {
          transplant(ids, node, node->right);
          node->right = cur->right;
          node->right->parent = node;
        }

        transplant(ids, cur, node);
        node->left = cur->left;
        node->left->parent = node;
        node->color = cur->color;
      }

      if (node_orig_color == RSPASM_IDENTIFIER_NODE_BLACK)
        delete_fixup(ids, temp);

      free(cur->name);
      free(cur);
      return 1;
    }
  }

  return 0;
}

void delete_fixup(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *node) {
  struct rspasm_identifier_node *cur;

  while (node != ids->root && node->color == RSPASM_IDENTIFIER_NODE_BLACK) {
    if (node == node->parent->left) {
      cur = node->parent->right;

      if (cur->color == RSPASM_IDENTIFIER_NODE_RED) {
        cur->color = RSPASM_IDENTIFIER_NODE_BLACK;
        node->parent->color = RSPASM_IDENTIFIER_NODE_RED;
        rotate_left(ids, node->parent);
        cur = node->parent->right;
      }

      if (cur->left->color == RSPASM_IDENTIFIER_NODE_BLACK &&
          cur->right->color == RSPASM_IDENTIFIER_NODE_BLACK) {
        cur->color = RSPASM_IDENTIFIER_NODE_RED;
        node = node->parent;
      }

      else {
        if (cur->right->color == RSPASM_IDENTIFIER_NODE_BLACK) {
          cur->left->color = RSPASM_IDENTIFIER_NODE_BLACK;
          cur->color = RSPASM_IDENTIFIER_NODE_RED;
          rotate_right(ids, cur);
          cur = node->parent->right;
        }

        cur->color = node->parent->color;
        node->parent->color = RSPASM_IDENTIFIER_NODE_BLACK;
        cur->right->color = RSPASM_IDENTIFIER_NODE_BLACK;
        rotate_left(ids, node->parent);
        node = ids->root;
      }
    }

    else {
      cur = node->parent->left;

      if (cur->color == RSPASM_IDENTIFIER_NODE_RED) {
        cur->color = RSPASM_IDENTIFIER_NODE_BLACK;
        node->parent->color = RSPASM_IDENTIFIER_NODE_RED;
        rotate_right(ids, node->parent);
        cur = node->parent->left;
      }

      if (cur->right->color == RSPASM_IDENTIFIER_NODE_BLACK &&
          cur->left->color == RSPASM_IDENTIFIER_NODE_BLACK) {
        cur->color = RSPASM_IDENTIFIER_NODE_RED;
        node = node->parent;
      }

      else {
        if (cur->left->color == RSPASM_IDENTIFIER_NODE_BLACK) {
          cur->right->color = RSPASM_IDENTIFIER_NODE_BLACK;
          cur->color = RSPASM_IDENTIFIER_NODE_RED;
          rotate_left(ids, cur);
          cur = node->parent->left;
        }

        cur->color = node->parent->color;
        node->parent->color = RSPASM_IDENTIFIER_NODE_BLACK;
        cur->left->color = RSPASM_IDENTIFIER_NODE_BLACK;
        rotate_right(ids, node->parent);
        node = ids->root;
      }
    }
  }

  node->color = RSPASM_IDENTIFIER_NODE_BLACK;
}

void fixup(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *node) {
  struct rspasm_identifier_node *cur;

  /* Rebalance the whole tree as needed */
  while (node->parent->color == RSPASM_IDENTIFIER_NODE_RED) {
    if (node->parent == node->parent->parent->left) {
      cur = node->parent->parent->right;

      if (cur->color == RSPASM_IDENTIFIER_NODE_RED) {
        node->parent->color = RSPASM_IDENTIFIER_NODE_BLACK;
        cur->color = RSPASM_IDENTIFIER_NODE_BLACK;
        node->parent->parent->color = RSPASM_IDENTIFIER_NODE_RED;
        node = node->parent->parent;
      }

      else {
        if (node == node->parent->right) {
          node = node->parent;
          rotate_left(ids, node);
        }

        node->parent->color = RSPASM_IDENTIFIER_NODE_BLACK;
        node->parent->parent->color = RSPASM_IDENTIFIER_NODE_RED;
        rotate_right(ids, node->parent->parent);
      }
    }

    else {
      cur = node->parent->parent->left;

      if (cur->color == RSPASM_IDENTIFIER_NODE_RED) {
        node->parent->color = RSPASM_IDENTIFIER_NODE_BLACK;
        cur->color = RSPASM_IDENTIFIER_NODE_BLACK;
        node->parent->parent->color = RSPASM_IDENTIFIER_NODE_RED;
        node = node->parent->parent;
      }

      else {
        if (node == node->parent->left) {
          node = node->parent;
          rotate_right(ids, node);
        }

        node->parent->color = RSPASM_IDENTIFIER_NODE_BLACK;
        node->parent->parent->color = RSPASM_IDENTIFIER_NODE_RED;
        rotate_left(ids, node->parent->parent);
      }
    }
  }

  ids->root->color = RSPASM_IDENTIFIER_NODE_BLACK;
}

void rotate_left(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *n) {
  struct rspasm_identifier_node *y = n->right;

  /* Turn y's left subtree into n's right subtree */
  n->right = y->left;

  if (y->left != ids->nil)
    y->left->parent = n;

  /* Link n's parent to y */
  y->parent = n->parent;

  if (n->parent == ids->nil)
    ids->root = y;
  else if (n == n->parent->left)
    n->parent->left = y;
  else
    n->parent->right = y;

  /* Put n on y's left */
  y->left = n;
  n->parent = y;
}

void rotate_right(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *n) {
  struct rspasm_identifier_node *y = n->left;

  /* Turn y's right subtree into n's left subtree */
  n->left = y->right;

  if (y->right != ids->nil)
    y->right->parent = n;

  /* Link n's parent to y */
  y->parent = n->parent;

  if (n->parent == ids->nil)
    ids->root = y;
  else if (n == n->parent->left)
    n->parent->left = y;
  else
    n->parent->right = y;

  /* Put n on y's right */
  y->right = n;
  n->parent = y;
}

void transplant(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *u, struct rspasm_identifier_node *v) {
  if (u->parent == ids->nil)
    ids->root = v;

  else if (u == u->parent->left)
    u->parent->left = v;

  else
    u->parent->right = v;

  v->parent = u->parent;
}

struct rspasm_identifier_node *tree_minimum(struct rspasm_identifiers *ids,
    struct rspasm_identifier_node *node) {

  while (node->left != ids->nil)
    node = node->left;

  return node;
}

