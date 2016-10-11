#!/bin/sh
# Install RISC-V toolchain that only requires the integer ISA, but supports
# the FGMP extension of RC/MC
#
# If an argument is given, as specific commit of the source code will be used
# for building the toolchain will be build. 
# The argument "tested" uses the last commit, the script was tested with.

TARGETDIR=`pwd`/../rv64ixfgmp

echo "Installing gcc for RISC-V without FPU but with FGMP (RV64IXfgmp) to $TARGETDIR"


##########################################
# download source code from git repository
##########################################

if [ "$1" = "tested" ]
then
  COMMIT=13f52d2
else
  COMMIT="$1"
fi
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
git checkout $COMMIT


########################################
# patch for support of FGMP instructions
########################################

# append opcode values to binutils/include/opcode/riscv-opc.h
cat >> binutils/include/opcode/riscv-opc.h <<EOF
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
EOF

# create temporary file with modifications to riscv-opc.c
cat > tmp <<EOF

/* Fine Grained Message Passing opcodes */
{"send",      "Xfgmp", "s,t", MATCH_SEND, MASK_SEND, match_opcode, 0},
{"cong",      "Xfgmp", "d",   MATCH_CONG, MASK_CONG, match_opcode, 0},
{"recv",      "Xfgmp", "d,s", MATCH_RECV, MASK_RECV, match_opcode, 0},
{"probe",     "Xfgmp", "d,s", MATCH_PROBE, MASK_PROBE, match_opcode, 0},
{"any",       "Xfgmp", "d",   MATCH_ANY, MASK_ANY, match_opcode, 0},
EOF

# modifiy opcode table for gas
sed -i -e '/^{"wfi",/rtmp' binutils/opcodes/riscv-opc.c
rm tmp


#################################
# build RISC-V
#################################

mkdir build
cd build
../configure --with-arch=RV64IXfgmp --prefix=$TARGETDIR --program-prefix=rv64ixfgmp-
make -j$(nproc)


