cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED ENV{VCPKG_PATH})
  message(FATAL_ERROR "Please clone vcpkg and set the VCPKG_PATH environment variable")
endif()

set(VCPKG_FEATURE_FLAGS versions)
set(VCPKG_TARGET_TRIPLET x64-linux)
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Dependencies and Includes - begin
project(gal)
find_package(PythonInterp 3.9 REQUIRED)
find_package(PythonLibs 3.9 REQUIRED)
find_package(Boost REQUIRED COMPONENTS python39)
if (Boost_FOUND)
    add_compile_definitions(BOOST_UUID_FORCE_AUTO_LINK)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
endif()
find_package(TBB CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)

# pngpp is a header only library
find_path(PNGPP_INCLUDE_DIRS "png++/color.hpp")

add_compile_definitions(TBB_SUPPRESS_DEPRECATED_MESSAGES)

include(GoogleTest)
find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include_directories(include)
# Dependencies and Includes - end

# GALCORE - begin
file(GLOB GALCORE_SRC "src/galcore/*.cpp")
add_library(galcore STATIC ${GALCORE_SRC})

target_link_libraries(galcore PUBLIC
    glm::glm
    tinyobjloader::tinyobjloader
    TBB::tbb
    spdlog::spdlog
    spdlog::spdlog_header_only)

target_include_directories(galcore PUBLIC ${PNGPP_INCLUDE_DIRS})
#GALCORE - end

#GALFUNC - begin
file(GLOB GALFUNC_SRC "src/galfunc/*.cpp")
# Share the object files between galfunc and pygalfunc
add_library(galfuncobjects OBJECT ${GALFUNC_SRC})
target_include_directories(galfuncobjects PUBLIC ${PYTHON_INCLUDE_DIRS})
target_link_libraries(galfuncobjects PUBLIC
    galcore
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    Boost::python39
    util
    dl)

add_library(galfunc STATIC $<TARGET_OBJECTS:galfuncobjects>)
# This will carry the dependencies and include dirs of galfuncobjects to
# any other library that uses galfunc
target_link_libraries(galfunc galfuncobjects)

# Python shared lib
add_library(pygalfunc SHARED $<TARGET_OBJECTS:galfuncobjects>)
set_target_properties(pygalfunc PROPERTIES
  SUFFIX .so
  PREFIX "")
target_link_libraries(pygalfunc galfuncobjects)
# Copy the python setup file
add_custom_command(TARGET pygalfunc
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/scripts/setup.py
  ${PROJECT_BINARY_DIR})
#GALFUNC - end

# GALVIEW - begin
file(GLOB GALVIEW_SRC "src/galview/*.cpp")
add_executable(galview ${GALVIEW_SRC})

target_link_libraries(galview PRIVATE
  galcore
  galfunc
  imgui::imgui
  GLEW::GLEW
  glfw
  freetype)

# Copy the shaders to bin dir
add_custom_command(TARGET galview
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/*.glsl ${PROJECT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/fonts/*.* ${PROJECT_BINARY_DIR}
)
#GALVIEW - end

# GALTEST - begin
add_compile_definitions(GAL_ASSET_DIR_STR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
enable_testing()
file (GLOB TEST_SRC "src/galtest/*.cpp")
add_executable(galtest ${TEST_SRC})
target_include_directories(galtest PRIVATE
    ${PYTHON_INCLUDE_DIRS})
target_link_libraries(galtest PRIVATE
    galcore
    galfunc
    GTest::gmock
    GTest::gtest
    GTest::gmock_main
    GTest::gtest_main)

gtest_discover_tests(galtest)
# GALTEST - end
