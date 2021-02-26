cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(VCPKG_TARGET_TRIPLET x64-linux)
set(CMAKE_PREFIX_PATH $ENV{VCPKG_PATH}/installed/${VCPKG_TARGET_TRIPLET})
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake)

project(galcore)

find_package(Boost REQUIRED)
if (Boost_FOUND)
    add_compile_definitions(BOOST_UUID_FORCE_AUTO_LINK)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

include_directories(include)

file(GLOB GALCORE_SRC "src/galcore/*.cpp")
add_library(galcore STATIC ${GALCORE_SRC})