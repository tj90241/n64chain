//
// rspasm/emitter.h: RSP assembler code/data generator.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSPASM_EMITTER_H
#define RSPASM_EMITTER_H
#define YYLTYPE RSPASMLTYPE
#define YYSTYPE RSPASMSTYPE
#include "opcodes.h"
#include "parser.h"
#include "lexer.h"

struct rspasm;

int rspasm_dmax_assert(struct rspasm *rspasm, const YYLTYPE *loc, long int where);

int rspasm_emit_byte(struct rspasm *rspasm, const YYLTYPE *loc, long int byte);
int rspasm_emit_half(struct rspasm *rspasm, const YYLTYPE *loc, long int half);
int rspasm_emit_word(struct rspasm *rspasm, const YYLTYPE *loc, long int word);

int rspasm_emit_instruction(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode);

int rspasm_emit_instruction_ri(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, long imm);

int rspasm_emit_instruction_ro(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, long offset, unsigned base);

int rspasm_emit_instruction_rrc0(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, unsigned rd);

int rspasm_emit_instruction_rri(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, unsigned rs, long imm);

int rspasm_emit_instruction_rrr(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rd, unsigned rs, unsigned rt);

int rspasm_emit_instruction_rrs(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rd, unsigned rt, unsigned sa);

int rspasm_emit_instruction_rrt(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rs, unsigned rt, int offset);

int rspasm_emit_instruction_vo_lwc2(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vt, unsigned element, long offset, unsigned base);

int rspasm_emit_instruction_vo_swc2(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vt, unsigned element, long offset, unsigned base);

int rspasm_emit_instruction_vv(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vd, unsigned delement, long vt, unsigned element);

int rspasm_emit_instruction_vvv(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vd, unsigned vs, unsigned vt, unsigned element);

#endif

