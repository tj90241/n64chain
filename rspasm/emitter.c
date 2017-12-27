//
// rspasm/emitter.c: RSP assembler code/data generator.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "emitter.h"
#include "rspasm.h"
#include <limits.h>
#include <stdint.h>
#include <string.h>

static int rspasm_emit_data_common(
  const struct rspasm *rspasm, const YYLTYPE *loc, size_t sz);

static int rspasm_emit_instruction_common(
  const struct rspasm *rspasm, const YYLTYPE *loc);

int rspasm_dmax_assert(struct rspasm *rspasm, const YYLTYPE *loc, long int where) {
  if (where < 0) {
    fprintf(stderr, "line %d: "
      ".dmax expression cannot be negative.\n",
      loc->first_line);

    return -1;
  }

  if (where > 0x1000) {
    fprintf(stderr, "line %d: "
      ".dmax expression must be less than or equal to 4096.\n",
      loc->first_line);

    return -1;
  }

  if ((rspasm->in_text && (long int) (rspasm->ihead - 4096) > where) ||
    (!rspasm->in_text && (long int) (rspasm->dhead - 0000) > where)) {
    fprintf(stderr, "line %d: "
      ".dmax assertion failed.\n", loc->first_line);

    return -1;
  }

  return 0;
}

int rspasm_emit_byte(struct rspasm *rspasm, const YYLTYPE *loc, long int byte) {
  uint8_t ubyte;

  if (rspasm_emit_data_common(rspasm, loc, sizeof(ubyte)))
    return -1;

  if (byte > UINT8_MAX || byte < INT8_MIN) {
    fprintf(stderr, "line %d: .byte: "
      "Value is out of range.\n", loc->first_line);

    return -1;
  }

  ubyte = byte;
  memcpy(rspasm->data + rspasm->dhead, &ubyte, sizeof(ubyte));
  rspasm->dhead += sizeof(ubyte);

  return 0;
}

int rspasm_emit_data_common(
  const struct rspasm *rspasm, const YYLTYPE *loc, size_t sz) {

  if (rspasm->in_text) {
    fprintf(stderr, "line %d: "
      "Attempted to emit data in .text section.\n", loc->first_line);

    return -1;
  }

  if (rspasm->dhead >= (0x1000 - sz)) {
    fprintf(stderr, "line %d: Value does not fit in DMEM.\n", loc->first_line);

    return -1;
  }

  return 0;
}

int rspasm_emit_half(struct rspasm *rspasm, const YYLTYPE *loc, long int half) {
  uint16_t uhalf;

  if (rspasm_emit_data_common(rspasm, loc, sizeof(uhalf)))
    return -1;

  if (half > UINT16_MAX || half < INT16_MIN) {
    fprintf(stderr, "line %d: .half: "
      "Value is out of range.\n", loc->first_line);

    return -1;
  }

  uhalf = htons(half);
  memcpy(rspasm->data + rspasm->dhead, &uhalf, sizeof(uhalf));
  rspasm->dhead += sizeof(uhalf);

  return 0;
}

int rspasm_emit_instruction_common(
  const struct rspasm *rspasm, const YYLTYPE *loc) {
  if (!rspasm->in_text) {
    fprintf(stderr, "line %d: "
      "Attempted to emit instruction in .data section.\n", loc->first_line);

    return -1;
  }

  if (rspasm->ihead >= 0x2000) {
    fprintf(stderr, "line %d: "
      "Instruction does not fit in IMEM.\n", loc->first_line);

    return -1;
  }

  return 0;
}

int rspasm_emit_instruction(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode) {
  uint32_t iw;

  if (rspasm_emit_instruction_common(rspasm, loc))
    return -1;

  iw = htonl(opcode);
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_ri(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, long imm) {
  uint32_t iw;

  if (rspasm_emit_instruction_common(rspasm, loc))
    return -1;

  if (opcode == LUI && (imm > UINT16_MAX || imm < INT16_MIN)) {
    fprintf(stderr, "line %d: "
      "Immediate value is out of range.\n", loc->first_line);

    return -1;
  }

  iw = htonl((opcode << 26) | (rt << 16) | (imm & 0xFFFF));
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_ro(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, long offset, unsigned base) {
  uint32_t iw;

  if (rspasm_emit_instruction_common(rspasm, loc))
    return -1;

  if (offset > 0xFFF || offset < 0) {
    fprintf(stderr, "line %d: "
      "Offset is out of range.\n", loc->first_line);

    return -1;
  }

  iw = htonl((opcode << 26) | (base << 21) | (rt << 16) | offset);
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_rrc0(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, unsigned rd) {
  uint32_t iw;

  if (rspasm_emit_instruction_common(rspasm, loc))
    return -1;

  if (opcode == MFC0 && rt == 0) {
    fprintf(stderr, "line %d: Instruction writes to $0.\n",
      loc->first_line);
  }

  iw = htonl((1 << 30) | (opcode << 21) | (rt << 16) | (rd << 11));
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_rri(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rt, unsigned rs, long imm) {
  uint32_t iw;

  // sign-extended: ADDI, ADDIU, ORI, SLTI, SLTIU
  if (opcode == ADDI || opcode == ADDIU ||
    opcode == ORI || opcode == SLTI || opcode == SLTIU) {
    if (imm > UINT16_MAX || imm < INT16_MIN) {
      fprintf(stderr, "line %d: "
        "Immediate value is out of range.\n", loc->first_line);

      return -1;
    }
  }

  // zero-extended: ANDI, XORI
  else {
    if (imm > UINT16_MAX || imm < 0) {
      fprintf(stderr, "line %d: "
        "Immediate value is out of range.\n", loc->first_line);

      return -1;
    }
  }

  iw = htonl((opcode << 26) | (rs << 21) | (rt << 16) | (imm & 0xFFFF));
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_rrr(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rd, unsigned rs, unsigned rt) {
  uint32_t iw;

  if (rd == 0) {
    fprintf(stderr, "line %d: Instruction writes to $0.\n",
      loc->first_line);
  }

  iw = htonl(opcode | (rd << 11) | (rt << 16) | (rs << 21));
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_rrs(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rd, unsigned rt, unsigned sa) {
  uint32_t iw;

  if (rd == 0) {
    fprintf(stderr, "line %d: Instruction writes to $0.\n",
      loc->first_line);
  }

  if (sa > 31) {
    fprintf(stderr, "line %d: Shift amount beyond specified range.\n",
      loc->first_line);
  }

  iw = htonl(opcode | (rd << 11) | (rt << 16) | ((sa & 0x1F) << 6));
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_rrt(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned rs, unsigned rt, int offset) {
  uint32_t soffset, iw;

  offset -= 0x1000;

  if (offset > 4096 || offset < -4096) {
    fprintf(stderr, "line %d: Offset greater than IMEM size.\n",
      loc->first_line);

    return -1;
  }

  offset -= (rspasm->ihead & 0xFFF) + 0x4;
  soffset = (unsigned short) ((short) offset >> 2);

  iw = htonl((opcode << 26) | (rs << 21) | (rt << 16) | (soffset & 0xFFFF));
  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_vo_lwc2(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vt, unsigned element, long offset, unsigned base) {
  unsigned offsetshift;
  uint32_t iw;

  if (rspasm_emit_instruction_common(rspasm, loc))
    return -1;

  if (vt == 0) {
    fprintf(stderr, "line %d: Instruction writes to $v0.\n",
      loc->first_line);
  }

  if (element > 0xF) {
    fprintf(stderr, "line %d: "
      "Element is out of range.\n", loc->first_line);

    return -1;
  }

  // TODO: LWV
  switch (opcode) {
    case LBV: offsetshift = 0; break;
    case LDV: offsetshift = 3; break;
    case LFV: offsetshift = 4; break;
    case LHV: offsetshift = 4; break;
    case LLV: offsetshift = 2; break;
    case LPV: offsetshift = 3; break;
    case LQV: offsetshift = 4; break;
    case LRV: offsetshift = 4; break;
    case LSV: offsetshift = 1; break;
    case LTV: offsetshift = 4; break;
    case LUV: offsetshift = 3; break;

    default:  offsetshift = 0; break;
  }

  // See if we shift any bits off - toss out a warning if so
  if ((offset >> offsetshift) << offsetshift != offset ||
      (offset >> (offsetshift + 0x6) != -1 &&
       offset >> (offsetshift + 0x6) != 0)) {
    fprintf(stderr, "line %d: "
      "Cannot properly represent specified offset.\n", loc->first_line);

    return -1;
  }

  iw = htonl((0x32 << 26) | (opcode << 11) | (vt << 16) | (base << 21) |
      (element << 7) | (offset >> offsetshift));

  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_vo_swc2(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vt, unsigned element, long offset, unsigned base) {
  unsigned offsetshift;
  uint32_t iw;

  if (rspasm_emit_instruction_common(rspasm, loc))
    return -1;

  if (element > 0xF) {
    fprintf(stderr, "line %d: "
      "Element is out of range.\n", loc->first_line);

    return -1;
  }

  // TODO: SWV
  switch (opcode) {
    case SBV: offsetshift = 0; break;
    case SDV: offsetshift = 3; break;
    case SFV: offsetshift = 4; break;
    case SHV: offsetshift = 4; break;
    case SLV: offsetshift = 2; break;
    case SPV: offsetshift = 3; break;
    case SQV: offsetshift = 4; break;
    case SRV: offsetshift = 4; break;
    case SSV: offsetshift = 1; break;
    case STV: offsetshift = 4; break;
    case SUV: offsetshift = 3; break;

    default:  offsetshift = 0; break;
  }

  // See if we shift any bits off - toss out a warning if so
  if ((offset >> offsetshift) << offsetshift != offset ||
      (offset >> (offsetshift + 0x6) != -1 &&
       offset >> (offsetshift + 0x6) != 0)) {
    fprintf(stderr, "line %d: "
      "Cannot properly represent specified offset.\n", loc->first_line);

    return -1;
  }

  iw = htonl((0x3A << 26) | (opcode << 11) | (vt << 16) | (base << 21) |
     (element << 7) | (offset >> offsetshift));

  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_vv(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vd, unsigned delement, long vt, unsigned element) {
  uint32_t iw;

  if (element > 0xF) {
    fprintf(stderr, "line %d: "
      "Source element is out of range.\n", loc->first_line);

    return -1;
  }

  if (delement > 0xF) {
    fprintf(stderr, "line %d: "
      "Destination element is out of range.\n", loc->first_line);

    return -1;
  }

  iw = htonl((0x25 << 25) | (vt << 16) | (vd << 6) |
     (element << 21) | (delement << 11) | opcode);

  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_instruction_vvv(struct rspasm *rspasm,
  const YYLTYPE *loc, enum rsp_opcode opcode,
  unsigned vd, unsigned vs, unsigned vt, unsigned element) {
  uint32_t iw;

  if (rspasm_emit_instruction_common(rspasm, loc))
    return -1;

  if (element > 0xF) {
    fprintf(stderr, "line %d: "
      "Element is out of range.\n", loc->first_line);

    return -1;
  }

  iw = htonl((0x25 << 25) | (vt << 16) | (vs << 11) | (vd << 6) |
     (element << 21) | opcode);

  memcpy(rspasm->data + rspasm->ihead, &iw, sizeof(iw));
  rspasm->ihead += sizeof(iw);

  return 0;
}

int rspasm_emit_word(struct rspasm *rspasm, const YYLTYPE *loc, long int word) {
  uint32_t uword;

  if (rspasm_emit_data_common(rspasm, loc, sizeof(uword)))
    return -1;

  if ((unsigned long int) word > UINT32_MAX || word < INT32_MIN) {
    fprintf(stderr, "line %d: .word: "
      "Value is out of range.\n", loc->first_line);

    return -1;
  }

  uword = htonl(word);
  memcpy(rspasm->data + rspasm->dhead, &uword, sizeof(uword));
  rspasm->dhead += sizeof(uword);

  return 0;
}

