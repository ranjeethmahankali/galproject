#!/bin/bash

CONFIG=${1-Release}
TARGET=${2-all}

mkdir -p build
cd build

echo "Generating compilation commands..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=$CONFIG ..
cp ./compile_commands.json ../

# Detect number of CPU cores in a cross-platform way
if [ "$(uname)" = "Darwin" ]; then
    # macOS uses sysctl to get CPU information
    nthreads=$(sysctl -n hw.ncpu)
elif [ "$(uname)" = "Linux" ]; then
    # Linux systems typically have nproc
    nthreads=$(nproc)
else
    # Fallback for other systems
    nthreads=4
fi

# Ensure we have a valid number
if [ -z "$nthreads" ] || [ "$nthreads" -lt 1 ]; then
    nthreads=1
fi

printf "Starting build using %s threads...\n" "$nthreads"
cmake --build . --config "$CONFIG" --target "$TARGET" -j "$nthreads"
