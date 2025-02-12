#!/bin/bash

CONFIG=${1-Release}
TARGET=${2-all}
NUM_THREADS=${3-12}


mkdir -p build
cd build

echo "Generating compilation commands..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
cp ./compile_commands.json ../

# Build with half the threads available on the computer.
nthreads=$(echo $(($(nproc) / 2)))
printf "Starting build using %s threads...\n" $nthreads
cmake --build . --config $CONFIG --target $TARGET -j $nthreads
