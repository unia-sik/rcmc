#!/bin/sh
# Modify riscv-binutils-gdb to support FGMP instructions


if [ "$(basename $(pwd))" != riscv-binutils-gdb ]
then
    echo "Execute in the riscv-binutils-gdb/ directory to add"
    echo "Fine Grained Message Passing Instructions to gas"
    exit 1
fi




################################################################################
# The opcode tables in riscv-opcodes do not support quad precision opcodes yet.
# Therefore we cannot use it to generate riscv-opc.h anymore 
# but have to modify it directly
#
## create new opcode file
#cd riscv-opcodes
#cat > opcodes-fgmp <<EOF

## Fine Grained Message Passing opcodes
#snd rs1 rs2   31..27=0 26..25=0                   14..12=0 11..7=4 6..2=0x1A 1..0=3
#rcvn rd rs1   31..27=0 26..25=0 24..20=4 19..15=0 14..12=4         6..2=0x1A 1..0=3
#rcvp rd rs1   31..27=0 26..25=0 24..20=5 19..15=0 14..12=4         6..2=0x1A 1..0=3
#bsf bimm12hi bimm12lo           24..20=4 19..15=0 14..12=1         6..2=0x1A 1..0=3
#bsnf bimm12hi bimm12lo          24..20=5 19..15=0 14..12=1         6..2=0x1A 1..0=3
#bre bimm12hi bimm12lo           24..20=6 19..15=0 14..12=1         6..2=0x1A 1..0=3
#brne bimm12hi bimm12lo          24..20=7 19..15=0 14..12=1         6..2=0x1A 1..0=3
#
## ready interface:
#srdy rs1      31..27=0 26..25=0 24..20=0          14..12=2 11..7=0 6..2=0x1A 1..0=3
#bnr rs1 bimm12hi bimm12lo       24..20=0          14..12=2         6..2=0x18 1..0=3
#
## deprecated:
#send rs1 rs2  31..27=0 26..25=0 14..12=0                   11..7=0 6..2=0x1A 1..0=3
#cong rd       31..27=0 26..25=0 24..20=0 19..15=0 14..12=1         6..2=0x1A 1..0=3
#recv rd rs1   31..27=0 26..25=0 24..20=0          14..12=4         6..2=0x1A 1..0=3
#probe rd rs1  31..27=0 26..25=0 24..20=0          14..12=5         6..2=0x1A 1..0=3
#any rd        31..27=0 26..25=0 24..20=0 19..15=0 14..12=7         6..2=0x1A 1..0=3
#@maxcid rd    31..20=0xC70               19..15=0 14..12=2         6..2=0x1C 1..0=3
#@cid    rd    31..20=0xC71               19..15=0 14..12=2         6..2=0x1C 1..0=3
#@nocdim rd    31..20=0xC72               19..15=0 14..12=2         6..2=0x1C 1..0=3
#EOF
#
## add opcode file to Makefile and run it
#sed -i -e 's/^ALL_OPCODES :=\(.*\)$/ALL_OPCODES :=\1 opcodes-fgmp/' Makefile
#make
#cd ..
#
################################################################################


# create temporary file with modifications to riscv-opc.c
cat > tmp <<EOF

/* Fine Grained Message Passing opcodes */
{"snd",       "I",   "s,t",  MATCH_SND, MASK_SND, match_opcode, 0},
{"srdy",      "I",   "s",  MATCH_SRDY, MASK_SRDY, match_opcode, 0},
{"rcvn",      "I",   "d",  MATCH_RCVN, MASK_RCVN, match_opcode, 0},
{"rcvp",      "I",   "d",  MATCH_RCVP, MASK_RCVP, match_opcode, 0},
{"bsf",       "I",   "p",  MATCH_BSF, MASK_BSF, match_opcode, 0},
{"bsnf",      "I",   "p",  MATCH_BSNF, MASK_BSNF, match_opcode, 0},
{"bre",       "I",   "p",  MATCH_BRE, MASK_BRE, match_opcode, 0},
{"brne",      "I",   "p",  MATCH_BRNE, MASK_BRNE, match_opcode, 0},
{"br",        "I",   "s,p",  MATCH_BR, MASK_BR, match_opcode, 0},
{"bnr",       "I",   "s,p",  MATCH_BNR, MASK_BNR, match_opcode, 0},
/* deprecated */
{"send",      "I", "s,t", MATCH_SEND, MASK_SEND, match_opcode, 0},
{"cong",      "I", "d",   MATCH_CONG, MASK_CONG, match_opcode, 0},
{"recv",      "I", "d,s", MATCH_RECV, MASK_RECV, match_opcode, 0},
{"probe",     "I", "d,s", MATCH_PROBE, MASK_PROBE, match_opcode, 0},
{"any",       "I", "d",   MATCH_ANY, MASK_ANY, match_opcode, 0},
{"maxcid",    "I", "d",   MATCH_MAXCID, MASK_MAXCID, match_opcode, 0},
{"cid",       "I", "d",   MATCH_CID, MASK_CID, match_opcode, 0},
{"nocdim",    "I", "d",   MATCH_NOCDIM, MASK_NOCDIM, match_opcode, 0},
EOF

# modifiy opcode table for gas
sed -i -e '/^{"wfi",/rtmp' opcodes/riscv-opc.c
rm tmp


cat > tmp <<EOF

#ifndef RISCV_ENCODING_H
#define MATCH_SND 0x5b
#define MASK_SND  0xfe007fff
#define MATCH_SRDY 0x106b
#define MASK_SRDY  0xfff07fff
#define MATCH_RCVN 0x205b
#define MASK_RCVN  0xfffff07f
#define MATCH_RCVP 0x305b
#define MASK_RCVP  0xfffff07f
#define MATCH_BSF 0x7b
#define MASK_BSF  0x1fff07f
#define MATCH_BSNF 0x107b
#define MASK_BSNF  0x1fff07f
#define MATCH_BRE 0x207b
#define MASK_BRE  0x1fff07f
#define MATCH_BRNE 0x307b
#define MASK_BRNE  0x1fff07f
#define MATCH_BR 0x407b
#define MASK_BR  0x1f0707f
#define MATCH_BNR 0x507b
#define MASK_BNR  0x1f0707f
#define MATCH_SEND 0x6b
#define MASK_SEND  0xfe007fff
#define MATCH_CONG 0x106b
#define MASK_CONG  0xfffff07f
#define MATCH_RECV 0x406b
#define MASK_RECV  0xfff0707f
#define MATCH_PROBE 0x506b
#define MASK_PROBE  0xfff0707f
#define MATCH_ANY 0x706b
#define MASK_ANY  0xfffff07f
#define MATCH_MAXCID 0xc7002073
#define MASK_MAXCID  0xfffff07f
#define MATCH_CID 0xc7102073
#define MASK_CID  0xfffff07f
#define MATCH_NOCDIM 0xc7202073
#define MASK_NOCDIM  0xfffff07f
#endif
#ifdef DECLARE_INSN
DECLARE_INSN(snd, MATCH_SND, MASK_SND)
DECLARE_INSN(rcvn, MATCH_RCVN, MASK_RCVN)
DECLARE_INSN(rcvp, MATCH_RCVP, MASK_RCVP)
DECLARE_INSN(bsf, MATCH_BSF, MASK_BSF)
DECLARE_INSN(bsnf, MATCH_BSNF, MASK_BSNF)
DECLARE_INSN(bre, MATCH_BRE, MASK_BRE)
DECLARE_INSN(brne, MATCH_BRNE, MASK_BRNE)
DECLARE_INSN(srdy, MATCH_SRDY, MASK_SRDY)
DECLARE_INSN(br, MATCH_BR, MASK_BR)
DECLARE_INSN(bnr, MATCH_BNR, MASK_BNR)
DECLARE_INSN(send, MATCH_SEND, MASK_SEND)
DECLARE_INSN(cong, MATCH_CONG, MASK_CONG)
DECLARE_INSN(recv, MATCH_RECV, MASK_RECV)
DECLARE_INSN(probe, MATCH_PROBE, MASK_PROBE)
DECLARE_INSN(any, MATCH_ANY, MASK_ANY)
DECLARE_INSN(maxcid, MATCH_MAXCID, MASK_MAXCID)
DECLARE_INSN(cid, MATCH_CID, MASK_CID)
DECLARE_INSN(nocdim, MATCH_NOCDIM, MASK_NOCDIM)
#endif
EOF

cat include/opcode/riscv-opc.h >> tmp
mv tmp include/opcode/riscv-opc.h
