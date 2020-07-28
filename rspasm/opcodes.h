//
// rspasm/opcodes.h: RSP opcodes.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef RSPASM_OPCODES_H
#define RSPASM_OPCODES_H

typedef enum rsp_opcode {
  BREAK = 0x0000000D,
  NOP = 0x00000000,
  VNOP = 0x4A000037,

  // OPCODE_R
  JALR = 0x09,
  JR = 0x08,

  // OPCODE_RRI
  ADDI = 0x08,
  ADDIU = 0x09,
  ANDI = 0x0C,
  LUI = 0x0F,
  ORI =  0x0D,
  SLL = 0x00,
  SLTI = 0x0A,
  SLTIU = 0x0B,
  SRA = 0x03,
  SRL = 0x02,
  XORI = 0x0E,

  // OPCODE_RO
  LB = 0x20,
  LBU = 0x24,
  LH = 0x21,
  LHU = 0x25,
  LW = 0x23,
  SB = 0x28,
  SH = 0x29,
  SW = 0x2B,

  // OPCODE_RT
  BGEZ = 0x01,
  BGEZAL = 0x11,
  BGTZ = 0x07,
  BLEZ = 0x06,
  BLTZ = 0x00,
  BLTZAL = 0x10,

  // OPCODE_RRT
  BEQ = 0x04,
  BNE = 0x05,

  // OPCODE_RRR
  ADD = 0x20,
  ADDU = 0x21,
  AND = 0x24,
  NOR = 0x27,
  OR = 0x25,
  SLLV = 0x04,
  SLT = 0x2A,
  SLTU = 0x2B,
  SRAV = 0x07,
  SRLV = 0x06,
  SUB = 0x22,
  SUBU = 0x23,
  XOR = 0x26,

  // OPCODE_RZ0
  MFC0 = 0x00,
  MTC0 = 0x04,

  // OPCODE_RZ2
  CFC2 = 0x02,
  CTC2 = 0x06,
  MFC2 = 0x00,
  MTC2 = 0x04,

  // OPCODE_T
  J = 0x02,
  JAL = 0x03,

  // OPCODE_VO_LWC2
  LBV = 0x00,
  LDV = 0x03,
  LFV = 0x09,
  LHV = 0x08,
  LLV = 0x02,
  LPV = 0x06,
  LQV = 0x04,
  LRV = 0x05,
  LSV = 0x01,
  LTV = 0x0B,
  LUV = 0x07,
  LWV = 0x0A,

  // OPCODE_VO_SWC2
  SBV = 0x00,
  SDV = 0x03,
  SFV = 0x09,
  SHV = 0x08,
  SLV = 0x02,
  SPV = 0x06,
  SQV = 0x04,
  SRV = 0x05,
  SSV = 0x01,
  STV = 0x0B,
  SUV = 0x07,
  SWV = 0x0A, /* Ultra64 documentation says 0x07?  */
              /* I'm assuming this a typo... ? */

  // OPCODE_VV
  VMOV = 0x33,
  VRCP = 0x30,
  VRCPH = 0x32,
  VRCPL = 0x31,
  VRSQ = 0x34,
  VRSQH = 0x36,
  VRSQL = 0x35,

  // OPCODE_VVV
  VABS = 0x13,
  VADD = 0x10,
  VADDC = 0x14,
  VAND = 0x28,
  VCH = 0x25,
  VCL = 0x24,
  VCR = 0x26,
  VEQ = 0x21,
  VGE = 0x23,
  VLT = 0x20,
  VMACF = 0x08,
  VMACQ = 0x0B,
  VMACU = 0x01,
  VMADH = 0x0F,
  VMADL = 0x0C,
  VMADM = 0x0D,
  VMADN = 0x0E,
  VMUDH = 0x07,
  VMUDL = 0x04,
  VMUDM = 0x05,
  VMUDN = 0x06,
  VMULF = 0x00,
  VMULQ = 0x03,
  VMULU = 0x01,
  VMRG = 0x27,
  VNAND = 0x29,
  VNE = 0x22,
  VNOR = 0x2B,
  VNXOR = 0x2D,
  VOR = 0x2A,
  VRNDN = 0x0A,
  VRNDP = 0x02,
  VSUB = 0x11,
  VSUBC = 0x15,
  VSAR = 0x1D,
  VXOR = 0x2C,

  // OPCODE_NS_VVV
  VACCB = 0x18,
  VADDB = 0x16,
  VEXTN = 0x3A,
  VEXTQ = 0x39,
  VEXTT = 0x38,
  VINSN = 0x3D,
  VINSQ = 0x3C,
  VINST = 0x3B,
  VSAC = 0x1B,
  VSAD = 0x1A,
  VSUBB = 0x17,
  VSUCB = 0x19,
  VSUM = 0x1C,
  VSUT = 0x12,

} opcode_t;

struct rsp_instruction {
  enum rsp_opcode op;

  unsigned r1;
  unsigned r2;
  unsigned r3;
  unsigned e;
};

#endif

