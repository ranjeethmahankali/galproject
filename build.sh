
CONFIG=${1-Release}
mkdir -p build
cd build
echo "Building with configuration ${CONFIG}"
cmake -DCMAKE_BUILD_TYPE=$CONFIG ..
cmake --build . --config $CONFIG -j 8