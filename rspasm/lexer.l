%{
//
// rspasm/lexer.l: RSP assembler lexer.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "opcodes.h"
#include "parser.h"
#include <limits.h>
#include <stdio.h>

#define YYLTYPE RSPASMLTYPE
#define YYSTYPE RSPASMSTYPE

#define YY_USER_ACTION \
  yylloc->first_line = yylloc->last_line; \
  yylloc->first_column = yylloc->last_column; \
  for(int i = 0; yytext[i] != '\0'; i++) { \
    if(yytext[i] == '\n') { \
      yylloc->last_line++; \
      yylloc->last_column = 0; \
    } else { \
      yylloc->last_column++; \
    } \
  }

static void get_constant_with_base(
    YYSTYPE *val, const char *text, int lineno, unsigned base);

static void get_identifier(YYSTYPE *val, const char *text, int lineno);
static enum rspasmtokentype get_register(YYSTYPE *val, const char *text,
    int lineno, enum rspasmtokentype register_token);

%}

/* Rules. */
%option prefix="rspasm"
%option reentrant bison-bridge
%option bison-locations
%option yylineno
%option noyywrap

%%

"add" { yylval->opcode = ADD; return OPCODE_RRR; }
"addi" { yylval->opcode = ADDI; return OPCODE_RRI; }
"addiu" { yylval->opcode = ADDIU; return OPCODE_RRI; }
"addu" { yylval->opcode = ADDU; return OPCODE_RRR; }
"and" { yylval->opcode = AND; return OPCODE_RRR; }
"andi" { yylval->opcode = ANDI; return OPCODE_RRI; }
"beq" { yylval->opcode = BEQ; return OPCODE_RRT; }
"bgez" { yylval->opcode = BGEZ; return OPCODE_RT; }
"bgezal" { yylval->opcode = BGEZAL; return OPCODE_RT; }
"bgtz" { yylval->opcode = BGTZ; return OPCODE_RT; }
"blez" { yylval->opcode = BLEZ; return OPCODE_RT; }
"bltz" { yylval->opcode = BLTZ; return OPCODE_RT; }
"bltzal" { yylval->opcode = BLTZAL; return OPCODE_RT; }
"bne" { yylval->opcode = BNE; return OPCODE_RRT; }
"break" { yylval->opcode = BREAK; return OPCODE; }
"cfc2" { yylval->opcode = CFC2; return OPCODE_RZ2; }
"ctc2" { yylval->opcode = CTC2; return OPCODE_RZ2; }
"j" { yylval->opcode = J; return OPCODE_T; }
"jal" { yylval->opcode = JAL; return OPCODE_T; }
"jalr" { yylval->opcode = JALR; return OPCODE_JALR; }
"jr" { yylval->opcode = JR; return OPCODE_R; }
"lb" { yylval->opcode = LB; return OPCODE_RO; }
"lbu" { yylval->opcode = LBU; return OPCODE_RO; }
"lbv" { yylval->opcode = LBV; return OPCODE_VO_LWC2; }
"ldv" { yylval->opcode = LDV; return OPCODE_VO_LWC2; }
"lfv" { yylval->opcode = LFV; return OPCODE_VO_LWC2; }
"lh" { yylval->opcode = LH; return OPCODE_RO; }
"lhu" { yylval->opcode = LHU; return OPCODE_RO; }
"lhv" { yylval->opcode = LHV; return OPCODE_VO_LWC2; }
"llv" { yylval->opcode = LLV; return OPCODE_VO_LWC2; }
"lpv" { yylval->opcode = LPV; return OPCODE_VO_LWC2; }
"lqv" { yylval->opcode = LQV; return OPCODE_VO_LWC2; }
"lrv" { yylval->opcode = LRV; return OPCODE_VO_LWC2; }
"lsv" { yylval->opcode = LSV; return OPCODE_VO_LWC2; }
"ltv" { yylval->opcode = LTV; return OPCODE_VO_LWC2; }
"lui" { yylval->opcode = LUI; return OPCODE_RI; }
"luv" { yylval->opcode = LUV; return OPCODE_VO_LWC2; }
"lw" { yylval->opcode = LW; return OPCODE_RO; }
"lwv" { yylval->opcode = LWV; return OPCODE_VO_LWC2; }
"mfc0" { yylval->opcode = MFC0; return OPCODE_RRC0; }
"mfc2" { yylval->opcode = MFC2; return OPCODE_RZ2E; }
"mtc0" { yylval->opcode = MTC0; return OPCODE_RRC0; }
"mtc2" { yylval->opcode = MTC2; return OPCODE_RZ2E; }
"nop" { yylval->opcode = NOP; return OPCODE; }
"nor" { yylval->opcode = NOR; return OPCODE_RRR; }
"or" { yylval->opcode = OR; return OPCODE_RRR; }
"ori" { yylval->opcode = ORI; return OPCODE_RRI; }
"sb" { yylval->opcode = SB; return OPCODE_RO; }
"sbv" { yylval->opcode = SBV; return OPCODE_VO_SWC2; }
"sdv" { yylval->opcode = SDV; return OPCODE_VO_SWC2; }
"sfv" { yylval->opcode = SFV; return OPCODE_VO_SWC2; }
"sh" { yylval->opcode = SH; return OPCODE_RO; }
"shv" { yylval->opcode = SHV; return OPCODE_VO_SWC2; }
"sll" { yylval->opcode = SLL; return OPCODE_RRS; }
"sllv" { yylval->opcode = SLLV; return OPCODE_RRR; }
"slt" { yylval->opcode = SLT; return OPCODE_RRR; }
"slti" { yylval->opcode = SLTI; return OPCODE_RRI; }
"sltiu" { yylval->opcode = SLTIU; return OPCODE_RRI; }
"sltu" { yylval->opcode = SLTU; return OPCODE_RRR; }
"slv" { yylval->opcode = SLV; return OPCODE_VO_SWC2; }
"spv" { yylval->opcode = SPV; return OPCODE_VO_SWC2; }
"sqv" { yylval->opcode = SQV; return OPCODE_VO_SWC2; }
"sra" { yylval->opcode = SRA; return OPCODE_RRS; }
"srav" { yylval->opcode = SRAV; return OPCODE_RRR; }
"srl" { yylval->opcode = SRL; return OPCODE_RRS; }
"srlv" { yylval->opcode = SRLV; return OPCODE_RRR; }
"srv" { yylval->opcode = SRV; return OPCODE_VO_SWC2; }
"ssv" { yylval->opcode = SSV; return OPCODE_VO_SWC2; }
"stv" { yylval->opcode = STV; return OPCODE_VO_SWC2; }
"sub" { yylval->opcode = SUB; return OPCODE_RRR; }
"subu" { yylval->opcode = SUBU; return OPCODE_RRR; }
"suv" { yylval->opcode = SUV; return OPCODE_VO_SWC2; }
"sw" { yylval->opcode = SW; return OPCODE_RO; }
"swv" { yylval->opcode = SWV; return OPCODE_VO_SWC2; }
"vabs" { yylval->opcode = VABS; return OPCODE_VVV; }
"vadd" { yylval->opcode = VADD; return OPCODE_VVV; }
"vaddc" { yylval->opcode = VADDC; return OPCODE_VVV; }
"vand" { yylval->opcode = VAND; return OPCODE_VVV; }
"vch" { yylval->opcode = VCH; return OPCODE_VVV; }
"vcl" { yylval->opcode = VCL; return OPCODE_VVV; }
"vcr" { yylval->opcode = VCR; return OPCODE_VVV; }
"veq" { yylval->opcode = VEQ; return OPCODE_VVV; }
"vge" { yylval->opcode = VGE; return OPCODE_VVV; }
"vlt" { yylval->opcode = VLT; return OPCODE_VVV; }
"vmacf" { yylval->opcode = VMACF; return OPCODE_VVV; }
"vmacq" { yylval->opcode = VMACQ; return OPCODE_VVV; }
"vmacu" { yylval->opcode = VMACU; return OPCODE_VVV; }
"vmadh" { yylval->opcode = VMADH; return OPCODE_VVV; }
"vmadl" { yylval->opcode = VMADL; return OPCODE_VVV; }
"vmadm" { yylval->opcode = VMADM; return OPCODE_VVV; }
"vmadn" { yylval->opcode = VMADN; return OPCODE_VVV; }
"vmov" { yylval->opcode = VMOV; return OPCODE_VV; }
"vmrg" { yylval->opcode = VMRG; return OPCODE_VVV; }
"vmudh" { yylval->opcode = VMUDH; return OPCODE_VVV; }
"vmudl" { yylval->opcode = VMUDL; return OPCODE_VVV; }
"vmudm" { yylval->opcode = VMUDM; return OPCODE_VVV; }
"vmudn" { yylval->opcode = VMUDN; return OPCODE_VVV; }
"vmulf" { yylval->opcode = VMULF; return OPCODE_VVV; }
"vmulq" { yylval->opcode = VMULQ; return OPCODE_VVV; }
"vmulu" { yylval->opcode = VMULU; return OPCODE_VVV; }
"vnand" { yylval->opcode = VNAND; return OPCODE_VVV; }
"vne" { yylval->opcode = VNE; return OPCODE_VVV; }
"vnop" { yylval->opcode = VNOP; return VOPCODE; }
"vnor" { yylval->opcode = VNOR; return OPCODE_VVV; }
"vnxor" { yylval->opcode = VNXOR; return OPCODE_VVV; }
"vor" { yylval->opcode = VOR; return OPCODE_VVV; }
"vrcp" { yylval->opcode = VRCP; return OPCODE_VV; }
"vrcph" { yylval->opcode = VRCPH; return OPCODE_VV; }
"vrcpl" { yylval->opcode = VRCPL; return OPCODE_VV; }
"vrndn" { yylval->opcode = VRNDN; return OPCODE_VVV; }
"vrndp" { yylval->opcode = VRNDP; return OPCODE_VVV; }
"vrsq" { yylval->opcode = VRSQ; return OPCODE_VV; }
"vrsqh" { yylval->opcode = VRSQH; return OPCODE_VV; }
"vrsql" { yylval->opcode = VRSQL; return OPCODE_VV; }
"vsar" { yylval->opcode = VSAR; return OPCODE_VVV; }
"vsaw" { yylval->opcode = VSAR; return OPCODE_VVV; }
"vsub" { yylval->opcode = VSUB; return OPCODE_VVV; }
"vsubc" { yylval->opcode = VSUBC; return OPCODE_VVV; }
"vxor" { yylval->opcode = VXOR; return OPCODE_VVV; }
"xor" { yylval->opcode = XOR; return OPCODE_RRR; }
"xori" { yylval->opcode = XORI; return OPCODE_RRI; }

".byte" { return DOTBYTE; }
".data" { return DOTDATA; }
".dmax" { return DOTDMAX; }
".half" { return DOTHALF; }
".set" { return DOTSET; }
".text" { return DOTTEXT; }
".unset" { return DOTUNSET; }
".word" { return DOTWORD; }

"," { return COMMA; }
":" { return COLON; }
"[" { return LEFT_BRACKET; }
"(" { return LEFT_PAREN; }
"]" { return RIGHT_BRACKET;}
")" { return RIGHT_PAREN; }

0[xX][0-9A-Fa-f]* {
  get_constant_with_base(yylval, yytext, yylineno, 16);
  return CONSTANT;
}

[1-9][0-9]* {
  get_constant_with_base(yylval, yytext, yylineno, 10);
  return CONSTANT;
}

0[0-7]* {
  get_constant_with_base(yylval, yytext, yylineno, 8);
  return CONSTANT;
}

[a-zA-Z_][a-zA-Z0-9_]* {
  get_identifier(yylval, yytext, yylineno);
  return IDENTIFIER;
}

"$at" { yylval->reg = 01; return SCALAR_REG; }
"$gp" { yylval->reg = 28; return SCALAR_REG; }
"$sp" { yylval->reg = 29; return SCALAR_REG; }
"$fp" { yylval->reg = 30; return SCALAR_REG; }
"$ra" { yylval->reg = 31; return SCALAR_REG; }

"$"v[0-9]+ {
  return get_register(yylval, yytext + 2, yylineno, VECTOR_REG);
}


"$"[0-9]+ {
  return get_register(yylval, yytext + 1, yylineno, SCALAR_REG);
}

"&" { return OP_AND; }
"~" { return OP_BNOT; }
"/" { return OP_DIVIDE; }
"<<" { return OP_LSHIFT; }
"-" { return OP_MINUS; }
"%" { return OP_MOD; }
"|" { return OP_OR; }
"+" { return OP_PLUS; }
">>" { return OP_RSHIFT; }
"*" { return OP_TIMES; }
"^" { return OP_XOR; }

#.*$
;.*$
"/*" {
  char c, c1;

  while (1) {
    while ((c = input(yyscanner)) != '*' && c != 0);

    if ((c1 = input(yyscanner)) != '/' && c != 0) {
      unput(c1);
      continue;
    }

    break;
  }
}

[ \t\n] {}

%%

void get_constant_with_base(YYSTYPE *val,
    const char *text, int lineno, unsigned base) {
  val->constant = strtol(text, NULL, base);

  if (val->constant == LONG_MIN || val->constant == LONG_MAX) {
    if (errno == ERANGE) {
      char error[80];

      snprintf(error, sizeof(error), "line %u: "
          "value is out of the assembler's supported range", lineno);

      yy_fatal_error(error, NULL);
    }
  }
}

void get_identifier(YYSTYPE *val, const char *text, int lineno) {
  size_t len = strlen(text);

  if (len > 31) {
    fprintf(stderr, "line %d: "
      "Identifier exceeds 31 characters in length.\n", lineno);

    len = 31;
  }

  memcpy(val->identifier, text, len);
  val->identifier[len] = '\0';
}

enum rspasmtokentype get_register(YYSTYPE *val, const char *text,
    int lineno, enum rspasmtokentype register_token) {
  long reg = strtol(text, NULL, 10);

  if (reg < 0 || reg > 31) {
    char error[80];

    snprintf(error, sizeof(error), "line %u: "
        "invalid register specified", lineno);

    yy_fatal_error(error, NULL);
  }

  val->reg = reg;
  return register_token;
}

