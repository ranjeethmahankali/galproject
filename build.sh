cd build
cmake -DCMAKE_BUILD_TYPE=$1 ..
cmake --config Release --build .