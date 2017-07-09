//
// rspasm/main.c: RSP assembler entry point.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#define YYLTYPE RSPASMLTYPE
#define YYSTYPE RSPASMSTYPE
#include "identifiers.h"
#include "parser.h"
#include "lexer.h"
#include "rspasm.h"
#include "symbols.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int assemble(FILE *in, FILE *out);
static void print_usage(const char *argv0);
static int safe_fwrite(FILE *out, const uint8_t *buf, size_t sz);

int assemble(FILE *in, FILE *out) {
  YY_BUFFER_STATE buf;
  yyscan_t scanner;

  struct rspasm_identifiers identifiers;
  struct rspasm rspasm;
  int status;

  if (rspasmlex_init(&scanner)) {
    fprintf(stderr, "Failed to initialize the assembler's lexer.\n");
    return EXIT_FAILURE;
  }

  if (rspasm_identifiers_create(&identifiers)) {
    fprintf(stderr, "Failed to initialize the identifier map.\n");
    rspasmlex_destroy(scanner);
    return EXIT_FAILURE;
  }

  rspasmset_extra(&rspasm, scanner);
  memset(&rspasm, 0x00, sizeof(rspasm));
  rspasm.identifiers = &identifiers;

  rspasm.dhead = 0x0000;
  rspasm.ihead = 0x1000;

  rspasm.first_pass = true;
  rspasm.in_text = true;

  buf = rspasm_create_buffer(in, YY_BUF_SIZE, scanner);
  rspasm_switch_to_buffer(buf, scanner);
  status = rspasmparse(scanner);

  // First pass was successful, go for a second.
  if (status != EXIT_FAILURE) {
    rspasm.dhead = 0x0000;
    rspasm.ihead = 0x1000;

    rspasm.first_pass = false;
    rspasm.in_text = true;

    if (rspasm_do_symbols_pass(&rspasm))
      status = EXIT_FAILURE;

    else {
      rewind(in);
      status = rspasmparse(scanner);

      if (status != EXIT_FAILURE) {
        if (safe_fwrite(out, rspasm.data, sizeof(rspasm.data)))
          status = EXIT_FAILURE;
      }

      rspasm_free_symbols(&rspasm);
    }
  }

  rspasm_identifiers_destroy(&identifiers);
  rspasm_delete_buffer(buf, scanner);
  rspasmlex_destroy(scanner);
  return status;
}

int main(int argc, const char *argv[]) {
  int status = EXIT_SUCCESS;
  FILE *input, *output;
  int i;

  if (argc < 2) {
    print_usage(argv[0]);
    return EXIT_SUCCESS;
  }

  for (i = 1, input = NULL, output = NULL; i < argc; i++) {

    // Not a filename.
    if (argv[i][0] == '-') {

      // Reading from stdin?
      if (argv[i][1] == '\0') {
        input = stdin;
        continue;
      }

      // Ensure it's a -X style argument.
      else if (argv[i][2] == '\0') {
        switch(argv[i][1]) {

          // -h: help
          case 'h':
            print_usage(argv[0]);
            i = argc;
            break;

          // -o: output file
          case 'o':
            if ((i + 1) >= argc) {
              fprintf(stderr, "-o requires a file path.\n");

              i = argc;
              status = EXIT_FAILURE;
              break;
            }

            else if (output) {
              fprintf(stderr, "Multiple outputs were specified.\n");

              i = argc;
              status = EXIT_FAILURE;
              break;
            }

            else if (!strcmp(argv[i + 1], "-")) {
              output = stdout;

              i++;
              break;
            }

            else if ((output = fopen(argv[i + 1], "wb")) == NULL) {
              fprintf(stderr, "Failed to open for writing: %s.\n", argv[i + 1]);

              i = argc;
              status = EXIT_FAILURE;
              break;
            }

            ++i;
            break;
        }
      }
    }

    // Got an input filename.
    else if (input != NULL) {
      fprintf(stderr, "Multiple inputs were specified.\n");

      i = argc;
      status = EXIT_FAILURE;
      break;
    }

    else if ((input = fopen(argv[i], "r")) == NULL) {
      fprintf(stderr, "Failed to open for reading: %s.\n", argv[i]);

      i = argc;
      status = EXIT_FAILURE;
      break;
    }
  }

  if (status != EXIT_FAILURE) {
    status = EXIT_FAILURE;

    if (input == NULL)
      fprintf(stderr, "An input file was not specified.\n");

    else if (output == NULL)
      fprintf(stderr, "An output file was not specified.\n");

    else
      status = assemble(input, output);
  }

  if (input && input != stdin)
    fclose(input);

  if (output && output != stdout)
    fclose(output);

  return status;
}

void print_usage(const char *argv0) {
  printf("Reality Signal Processor Assembler (rspasm)\n"
         "Copyright (C) 2014-15, Tyler J. Stachecki.\n\n"

         "Usage: %s [options] <file>\n"
         "  -h                          : Display this menu\n"
         "  -o <file>                   : Output to named file\n"

    , argv0);
}

int safe_fwrite(FILE *out, const uint8_t *buf, size_t sz) {
  size_t i, amt;

  for (i = 0; i < sz; i += amt) {
    amt = fwrite(buf + i, 1, sz - i, out);

    if (ferror(out)) {
      fprintf(stderr, "Failed to write to output stream.\n");
      return 1;
    }
  }

  return 0;
}

