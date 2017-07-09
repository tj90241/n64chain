%{
//
// rspasm/parser.y: RSP assembler parser.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#define YYDEBUG 0
int yydebug = 0;
%}

// Global state: be gone.
%define api.pure full
%define api.prefix {rspasm}
%lex-param { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%locations

%union {
  char identifier[32];
  long int constant;
  enum rsp_opcode opcode;
  unsigned reg;
}

%code requires {
#include "identifiers.h"
#include "opcodes.h"
#include "rspasm.h"
#include "symbols.h"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void *yyscan_t;
#endif
}

%code {
#include "emitter.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

// Raised on a parser error.
int yyerror(YYLTYPE *yylloc, yyscan_t scanner, const char *error) {
  //struct rspasm *rspasm = rspasmget_extra(scanner);
  fprintf(stderr, "line %d: %s\n", yylloc->first_line, error);
  return 0;
}
}

%token COLON
%token COMMA
%token CONSTANT
%token DIVIDE
%token DOTBYTE
%token DOTDATA
%token DOTDMAX
%token DOTHALF
%token DOTSET
%token DOTTEXT
%token DOTUNSET
%token DOTWORD
%token IDENTIFIER
%token LEFT_BRACKET
%token LEFT_PAREN
%token OPCODE
%token OPCODE_JALR //
%token OPCODE_R //
%token OPCODE_RRC0
%token OPCODE_RI
%token OPCODE_RO
%token OPCODE_RR //
%token OPCODE_RRI
%token OPCODE_RRR
%token OPCODE_RRS //
%token OPCODE_RRT //
%token OPCODE_RT //
%token OPCODE_RZ2 //
%token OPCODE_RZ2E //
%token OPCODE_T //
%token OPCODE_VO_LWC2 //
%token OPCODE_VO_SWC2 //
%token OPCODE_VV //
%token OPCODE_VVV //
%token OP_AND
%token OP_BNOT
%token OP_DIVIDE
%token OP_LSHIFT
%token OP_MINUS
%token OP_MOD
%token OP_OR
%token OP_PLUS
%token OP_RSHIFT
%token OP_TIMES
%token OP_XOR
%token RIGHT_BRACKET
%token RIGHT_PAREN
%token SCALAR_REG
%token VECTOR_REG
%token VOPCODE

%left OP_AND
%left OP_DIVIDE
%left OP_LSHIFT
%left OP_MINUS
%left OP_MOD
%left OP_OR
%left OP_PLUS
%left OP_RSHIFT
%left OP_TIMES
%left OR_XOR

%right OP_BNOT

%type <constant> constexpr expr CONSTANT
%type <identifier> IDENTIFIER
%type <opcode> OPCODE OPCODE_RI OPCODE_RO OPCODE_RRC0 OPCODE_RRI OPCODE_RRR
               VOPCODE
%type <reg> SCALAR_REG VECTOR_REG scalar_register

%%

toplevel:
  | program;

program:
    instruction
  | program instruction
  ;

instruction:
    directive
  | label
  | scalar_instruction
  | vector_instruction
  ;

directive:
    DOTBYTE expr {
      if (rspasm_emit_byte(rspasmget_extra(scanner), &yyloc, $2))
        return EXIT_FAILURE;
    }

  | DOTHALF expr {
      if (rspasm_emit_half(rspasmget_extra(scanner), &yyloc, $2))
        return EXIT_FAILURE;
    }

  | DOTWORD expr {
      if (rspasm_emit_word(rspasmget_extra(scanner), &yyloc, $2))
        return EXIT_FAILURE;
    }

  | DOTSET IDENTIFIER COMMA constexpr {
      struct rspasm *rspasm = rspasmget_extra(scanner);

      if (!rspasm->first_pass) {
        rspasm_identifiers_set(rspasm->identifiers, $2, $4,
            RSPASM_IDENTIFIER_NODE_INT);
      }
    }

  | DOTSET IDENTIFIER COMMA SCALAR_REG {
      struct rspasm *rspasm = rspasmget_extra(scanner);

      if (!rspasm->first_pass) {
        rspasm_identifiers_set(rspasm->identifiers, $2, $4,
            RSPASM_IDENTIFIER_NODE_REG);
      }
    }

  | DOTSET IDENTIFIER COMMA VECTOR_REG {
      struct rspasm *rspasm = rspasmget_extra(scanner);

      if (!rspasm->first_pass) {
        rspasm_identifiers_set(rspasm->identifiers, $2, $4,
            RSPASM_IDENTIFIER_NODE_VREG);
      }
    }

  | DOTUNSET IDENTIFIER {
      struct rspasm *rspasm = rspasmget_extra(scanner);

      if (!rspasm->first_pass) {
        if (!rspasm_identifiers_unset(rspasm->identifiers, $2)) {
          printf("Warning: %s was undefined at .unset\n", $2);
        }
      }
  }

  | DOTDATA { ((struct rspasm *) rspasmget_extra(scanner))->in_text = false; }
  | DOTTEXT { ((struct rspasm *) rspasmget_extra(scanner))->in_text = true; }

  | DOTDATA constexpr
  | DOTDMAX constexpr {
      if (rspasm_dmax_assert(rspasmget_extra(scanner), &yyloc, $2))
        return EXIT_FAILURE;
    }
  ;

label:
  IDENTIFIER COLON {
    struct rspasm *rspasm = rspasmget_extra(scanner);
    uint32_t addr = rspasm->in_text ? rspasm->ihead : rspasm->dhead;

    if (rspasm->first_pass) {
      if (rspasm_add_symbol(rspasm, $1, addr))
        return EXIT_FAILURE;
    }
  };

scalar_instruction:
    OPCODE {
      if (rspasm_emit_instruction(rspasmget_extra(scanner), &yyloc, $1))
        return EXIT_FAILURE;
    }

    | OPCODE_RI scalar_register COMMA expr {
      if (rspasm_emit_instruction_ri(
        rspasmget_extra(scanner), &yyloc, $1, $2, $4))
        return EXIT_FAILURE;
    }

    | OPCODE_RO scalar_register COMMA expr LEFT_PAREN scalar_register RIGHT_PAREN {
      if (rspasm_emit_instruction_ro(
        rspasmget_extra(scanner), &yyloc, $1, $2, $4, $6))
        return EXIT_FAILURE;
    }

    | OPCODE_RRC0 SCALAR_REG COMMA scalar_register {
      if (rspasm_emit_instruction_rrc0(
        rspasmget_extra(scanner), &yyloc, $1, $2, $4))
        return EXIT_FAILURE;
    }

    | OPCODE_RRI SCALAR_REG COMMA scalar_register COMMA expr {
      if (rspasm_emit_instruction_rri(
        rspasmget_extra(scanner), &yyloc, $1, $2, $4, $6))
        return EXIT_FAILURE;
    }

    | OPCODE_RRR SCALAR_REG COMMA scalar_register COMMA scalar_register {
      if (rspasm_emit_instruction_rrr(
        rspasmget_extra(scanner), &yyloc, $1, $2, $4, $6))
        return EXIT_FAILURE;
    }

  ;

scalar_register:
    SCALAR_REG {
      $$ = $1;
    }

  | IDENTIFIER {
      const struct rspasm *rspasm = rspasmget_extra(scanner);

      if (!rspasm->first_pass) {
        enum rspasm_identifier_node_type type;
        int32_t reg;

        if (!rspasm_identifiers_get(rspasm->identifiers, $1, &reg, &type)) {
          fprintf(stderr, "line %u: unknown symbol or identifier: %s\n",
              @1.first_line, $1);

          YYERROR;
        }

        if (type != RSPASM_IDENTIFIER_NODE_REG) {
          fprintf(stderr, "line %u: %s: expected a scalar register\n",
              @1.first_line, $1);

          YYERROR;
        }

        $$ = reg;
      }
    }
  ;

vector_instruction:
    VOPCODE {
      if (rspasm_emit_instruction(rspasmget_extra(scanner), &yyloc, $1))
        return EXIT_FAILURE;
    }
  ;

constexpr:
    LEFT_PAREN constexpr RIGHT_PAREN { $$ = $2; }
  | CONSTANT { $$ = $1; }

  | OP_BNOT constexpr { $$ = ~$2; }
  | constexpr OP_AND constexpr { $$ = $1 & $3; }
  | constexpr OP_OR constexpr { $$ = $1 | $3; }
  | constexpr OR_XOR constexpr { $$ = $1 ^ $3; }
  | constexpr OP_LSHIFT constexpr { $$ = $1 << $3; }
  | constexpr OP_RSHIFT constexpr { $$ = $1 >> $3; }
  | constexpr OP_TIMES constexpr { $$ = $1 * $3; }
  | constexpr OP_DIVIDE constexpr { $$ = $1 / $3; }
  | constexpr OP_MOD constexpr { $$ = $1 % $3; }
  | constexpr OP_PLUS constexpr { $$ = $1 | $3; }
  | constexpr OP_MINUS constexpr { $$ = $1 - $3; }

  | OP_MINUS constexpr %prec OP_BNOT { $$ = -$2; }
  | OP_PLUS constexpr %prec OP_BNOT { $$ = +$2; }
  ;

expr:
    LEFT_PAREN expr RIGHT_PAREN { $$ = $2; }
  | CONSTANT { $$ = $1; }

  | IDENTIFIER {
      const struct rspasm *rspasm = rspasmget_extra(scanner);

      if (rspasm->first_pass)
        $$ = 0;

      else {
        uint32_t addr;
        int32_t val;

        if (rspasm_get_symbol_address(rspasm, $1, &addr)) {
          enum rspasm_identifier_node_type type;

          if (!rspasm_identifiers_get(rspasm->identifiers, $1, &val, &type)) {
            fprintf(stderr, "line %u: unknown symbol or identifier: %s\n",
                @1.first_line, $1);

            YYERROR;
          }

          if (type != RSPASM_IDENTIFIER_NODE_INT) {
            fprintf(stderr, "line %u: %s is a register; expected a constant\n",
                @1.first_line, $1);

            YYERROR;
          }

          $$ = val;
        }

        else {
          $$ = addr;
        }
      }
    }

  | OP_BNOT expr { $$ = ~$2; }
  | expr OP_AND expr { $$ = $1 & $3; }
  | expr OP_OR expr { $$ = $1 | $3; }
  | expr OR_XOR expr { $$ = $1 ^ $3; }
  | expr OP_LSHIFT expr { $$ = $1 << $3; }
  | expr OP_RSHIFT expr { $$ = $1 >> $3; }
  | expr OP_TIMES expr { $$ = $1 * $3; }
  | expr OP_DIVIDE expr { $$ = $1 / $3; }
  | expr OP_MOD expr { $$ = $1 % $3; }
  | expr OP_PLUS expr { $$ = $1 | $3; }
  | expr OP_MINUS expr { $$ = $1 - $3; }

  | OP_MINUS expr %prec OP_BNOT { $$ = -$2; }
  | OP_PLUS expr %prec OP_BNOT { $$ = +$2; }
  ;

%%

