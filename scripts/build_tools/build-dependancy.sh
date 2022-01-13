#!/bin/bash
set -e
LOG_FILE=$1
echo > $LOG_FILE
if [ -d "llvm-8.0.0/install" ]; then
  echo "LLVM 8.0.0 already installed"
else
  if ! [ -d "llvm-8.0.0" ]; then
    echo Downloading LLVM 8.0.0 ...
    git clone https://github.com/Changxi-Liu/llvm-8.0.0.git
  fi
  echo Building LLVM 8.0.0 ...
  cd llvm-8.0.0
  mkdir -p build && cd build
  ../compiling.sh >> $LOG_FILE 2>&1
  make -j4 install >> $LOG_FILE 2>&1
fi

if [ -d "papi-6.0.0/install" ]; then
  echo "PAPI 6.0.0 already installed"
else
  if ! [ -f "papi-6.0.0.tar.gz" ]; then
    echo Downloading PAPI 6.0.0 ...
    wget http://icl.utk.edu/projects/papi/downloads/papi-6.0.0.tar.gz
  fi
  echo Building PAPI 6.0.0 ...
  tar xvzf papi-6.0.0.tar.gz >> $LOG_FILE 2>&1
  cd papi-6.0.0/src
  CC=gcc CXX=g++ F77=gfortran ./configure --prefix=`pwd`/../install >> $LOG_FILE 2>&1
  make -j4 >> $LOG_FILE 2>&1
  make install >> $LOG_FILE 2>&1
fi

