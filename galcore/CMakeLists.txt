cmake_minimum_required(VERSION 3.16.0)

file(GLOB GALCORE_SRC "*.cpp")
add_library(galcore STATIC ${GALCORE_SRC})

target_link_libraries(galcore PUBLIC
  ${OPENMESH_LIBRARIES}
  glm::glm
  TBB::tbb
  spdlog::spdlog
  spdlog::spdlog_header_only
  CGAL::CGAL
  Eigen3::Eigen
)

target_include_directories(galcore PUBLIC
  "include/"
  ${OPENMESH_INCLUDE_DIRS}
  ${PNGPP_INCLUDE_DIRS})
