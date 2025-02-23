cmake_minimum_required(VERSION 3.20.0)

add_compile_definitions(GAL_ASSET_DIR_STR="${PROJECT_SOURCE_DIR}/assets")
find_package(Catch2 CONFIG REQUIRED)
file (GLOB TEST_SRC "*.cpp")
add_executable(galtest ${TEST_SRC})
target_include_directories(galtest PRIVATE
  "include/"
  ${PYTHON_INCLUDE_DIRS}
)
target_link_libraries(galtest PRIVATE
    galcore
    galfunc
    Catch2::Catch2
    Catch2::Catch2WithMain)

if (WIN32)
  set_property(TARGET galtest PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(CTest)
include(Catch)
catch_discover_tests(galtest)
