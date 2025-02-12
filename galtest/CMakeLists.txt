cmake_minimum_required(VERSION 3.20.0)

add_compile_definitions(GAL_ASSET_DIR_STR="${PROJECT_SOURCE_DIR}/assets")
enable_testing()
file (GLOB TEST_SRC "*.cpp")
add_executable(galtest ${TEST_SRC})
target_include_directories(galtest PRIVATE
  "include/"
  ${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(galtest PRIVATE
    galcore
    galfunc
    GTest::gmock
    GTest::gtest
    GTest::gmock_main
    GTest::gtest_main)

if (WIN32)
  set_property(TARGET galtest PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

gtest_discover_tests(galtest)
