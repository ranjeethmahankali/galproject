cmake_minimum_required(VERSION 3.16.0)

add_compile_definitions(GAL_ASSET_DIR_STR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
enable_testing()
file (GLOB TEST_SRC "*.cpp")
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
