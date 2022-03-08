CONFIG=${1-build}
TARGET=${2-all}
NUM_THREADS=${3-4}
mkdir -p $CONFIG
cd $CONFIG
echo "Building target '${TARGET}' in ${CONFIG} configuration"

# This is the default version.
if [ $CONFIG == "build" ]; then
    echo "Generating compilation commands..."
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
    cp ./compile_commands.json ../
else
    cmake -DCMAKE_BUILD_TYPE=$CONFIG ..
    printf "Starting build using %s threads...\n" $NUM_THREADS
    cmake --build . --config $CONFIG --target $TARGET -j $NUM_THREADS
fi
