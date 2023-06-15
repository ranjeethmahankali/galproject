cmake_minimum_required(VERSION 3.16.0)

file(GLOB GALFUNC_SRC "*.cpp")
# Share the object files between galfunc and pygalfunc
add_library(galfuncobjects OBJECT ${GALFUNC_SRC})
target_include_directories(galfuncobjects PUBLIC
  "include/"
  ${PYTHON_INCLUDE_DIRS})
target_link_libraries(galfuncobjects PUBLIC
    galcore
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    pybind11::headers
    util
    dl)

add_library(galfunc STATIC $<TARGET_OBJECTS:galfuncobjects>)
# This will carry the dependencies and include dirs of galfuncobjects to
# any other library that uses galfunc
target_link_libraries(galfunc galfuncobjects)

# Python shared lib
pybind11_add_module(pygalfunc SHARED $<TARGET_OBJECTS:galfuncobjects>)
set_target_properties(pygalfunc PROPERTIES
  SUFFIX .so
  PREFIX "")
target_link_libraries(pygalfunc PUBLIC galfuncobjects)
# Copy the python setup file
add_custom_command(TARGET pygalfunc
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/scripts/setup.py
  ${CMAKE_CURRENT_BINARY_DIR})
# Install the python package so the latest bindings are available
add_custom_command(TARGET pygalfunc
  POST_BUILD
  COMMAND pip3 install -e ${CMAKE_CURRENT_BINARY_DIR}
)
