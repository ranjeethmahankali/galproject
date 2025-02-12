#!/bin/bash

CONFIG=${1-Release}
TARGET=${2-all}

mkdir -p build
cd build

echo "Generating compilation commands..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=$CONFIG ..
cp ./compile_commands.json ../

# Build with half the threads available on the computer, ensuring at least 1 thread is used.
nthreads=$(($(nproc) / 2))
if [ "$nthreads" -lt 1 ]; then
    nthreads=1
fi

printf "Starting build using %s threads...\n" "$nthreads"
cmake --build . --config "$CONFIG" --target "$TARGET" -j "$nthreads"
