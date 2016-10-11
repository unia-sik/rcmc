#!/bin/sh
# Modify riscv-tools source code to support FGMP instructions
#
# Usage:
# 1. Execute this script in the root directory (`riscv-tools/`) of your
#    riscv-tools git repository.
# 2. Re-build the toolset by executing RISC-V's build.sh


if [ ! -e riscv-opcodes/Makefile ] || [ ! -e riscv-gnu-toolchain/binutils/opcodes/riscv-opc.c ]
then
    echo "$1 must be executed in the root directory of the riscv-tools repository"
    exit
fi


# create new opcode file
cd riscv-opcodes
cat > opcodes-fgmp <<EOF
# Fine Grained Message Passing opcodes
send rs1 rs2  31..27=0 26..25=0 14..12=0                   11..7=0 6..2=0x1A 1..0=3
cong rd       31..27=0 26..25=0 24..20=0 19..15=0 14..12=1         6..2=0x1A 1..0=3
recv rd rs1   31..27=0 26..25=0 24..20=0          14..12=4         6..2=0x1A 1..0=3
probe rd rs1  31..27=0 26..25=0 24..20=0          14..12=5         6..2=0x1A 1..0=3
any rd        31..27=0 26..25=0 24..20=0 19..15=0 14..12=7         6..2=0x1A 1..0=3
@maxcid rd    31..20=0xC70               19..15=0 14..12=2         6..2=0x1C 1..0=3
@cid    rd    31..20=0xC71               19..15=0 14..12=2         6..2=0x1C 1..0=3
@nocdim rd    31..20=0xC72               19..15=0 14..12=2         6..2=0x1C 1..0=3
EOF

# add opcode file to Makefile and run it
sed -i -e 's/^ALL_OPCODES :=\(.*\)$/ALL_OPCODES :=\1 opcodes-fgmp/' Makefile
make
cd ..

# create temporary file with modifications to riscv-opc.c
cat > tmp <<EOF

/* Fine Grained Message Passing opcodes */
{"send",      "Xfgmp", "s,t", MATCH_SEND, MASK_SEND, match_opcode, 0},
{"cong",      "Xfgmp", "d",   MATCH_CONG, MASK_CONG, match_opcode, 0},
{"recv",      "Xfgmp", "d,s", MATCH_RECV, MASK_RECV, match_opcode, 0},
{"probe",     "Xfgmp", "d,s", MATCH_PROBE, MASK_PROBE, match_opcode, 0},
{"any",       "Xfgmp", "d",   MATCH_ANY, MASK_ANY, match_opcode, 0},
{"maxcid",    "Xfgmp", "d",   MATCH_MAXCID, MASK_MAXCID, match_opcode, 0},
{"cid",       "Xfgmp", "d",   MATCH_CID, MASK_CID, match_opcode, 0},
{"nocdim",    "Xfgmp", "d",   MATCH_NOCDIM, MASK_NOCDIM, match_opcode, 0},
EOF

# modifiy opcode table for gas
sed -i -e '/^{"wfi",/rtmp' riscv-gnu-toolchain/binutils/opcodes/riscv-opc.c
rm tmp

