#!/bin/sh
# Install GHDL from source code (slow mcode version)


TARGETDIR=$(pwd)/../ghdl


if [ $# -eq 0 ]
then
  echo "Installing tested commit of GHDL to $TARGETDIR"
  COMMIT=af74db16
  # 2017-08-27
elif [ "$1" = "latest" ]
then
  echo "Installing latest commit of GHDL to $TARGETDIR"
  COMMIT=master
else
  echo "Installing commit $1 of GHDL to $TARGETDIR"
  COMMIT="$1"
fi

rm -rf ghdl
git clone https://github.com/ghdl/ghdl.git
cd ghdl
git checkout $COMMIT
./configure --prefix=$TARGETDIR --with-llvm-config
make
make install
cd ..