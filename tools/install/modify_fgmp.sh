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
