cmake_minimum_required(VERSION 3.20.0)

file(GLOB GALCORE_SRC "*.cpp")
add_library(galcore STATIC ${GALCORE_SRC})

target_include_directories(galcore PUBLIC
  "include/"
  ${PNGPP_INCLUDE_DIRS})

target_link_libraries(galcore PUBLIC
  OpenMeshCore
  OpenMeshTools
  glm::glm
  TBB::tbb
  spdlog::spdlog
  spdlog::spdlog_header_only)

if (WIN32)
  set_property(TARGET galcore PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
