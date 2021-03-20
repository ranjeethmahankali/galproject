cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VCPKG_TARGET_TRIPLET x64-linux)
set(CMAKE_PREFIX_PATH $ENV{VCPKG_PATH}/installed/${VCPKG_TARGET_TRIPLET})
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

find_package(glm CONFIG REQUIRED)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui CONFIG REQUIRED)

include(GoogleTest)
find_package(GTest CONFIG REQUIRED)

include_directories(include)
# Dependencies and Includes - end

# GALCORE - begin
file(GLOB GALCORE_SRC "src/galcore/*.cpp")
add_library(galcore STATIC ${GALCORE_SRC})

target_link_libraries(galcore PUBLIC glm::glm)
#GALCORE - end

#GALFUNC - begin
file(GLOB GALFUNC_SRC "src/galfunc/*.cpp")
add_library(galfunc STATIC ${GALFUNC_SRC})
target_link_libraries(galfunc PUBLIC
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    Boost::python39)
target_include_directories(galfunc PRIVATE ${PYTHON_INCLUDE_DIRS})

add_library(pygalfunc SHARED ${GALFUNC_SRC})
set_target_properties(pygalfunc PROPERTIES SUFFIX .so)
set_target_properties(pygalfunc PROPERTIES PREFIX "")
target_link_libraries(pygalfunc PRIVATE
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    galcore
    Boost::python39)
target_include_directories(pygalfunc PRIVATE ${PYTHON_INCLUDE_DIRS})
#GALFUNC - end

# GALVIEW - begin
file(GLOB GALVIEW_SRC "src/galview/*.cpp")
add_executable(galview ${GALVIEW_SRC})

target_link_libraries(galview PRIVATE
  galcore
  galfunc
  imgui::imgui
  GLEW::GLEW
  glfw)

# Copy the shaders to bin dir
add_custom_command(TARGET galview
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/*.glsl ${PROJECT_BINARY_DIR}
)
#GALVIEW - end

# GALTEST - begin
enable_testing() 
file (GLOB TEST_SRC "src/galtest/*.cpp")
add_executable(galtest ${TEST_SRC})
add_dependencies(galtest
    galcore)

target_link_libraries(galtest PRIVATE
    galcore
    galfunc
    GTest::gmock
    GTest::gtest
    GTest::gmock_main
    GTest::gtest_main)

gtest_discover_tests(galtest)
# GALTEST - end
