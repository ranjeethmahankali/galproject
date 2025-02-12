cmake_minimum_required(VERSION 3.20.0)

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
    pybind11::embed)

if (NOT WIN32)
    target_link_libraries(galfuncobjects PUBLIC util dl)
endif()

add_library(galfunc STATIC $<TARGET_OBJECTS:galfuncobjects>)
# This will carry the dependencies and include dirs of galfuncobjects to
# any other library that uses galfunc
target_link_libraries(galfunc galfuncobjects)

if (WIN32)
  set_property(TARGET galfunc PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property(TARGET galfuncobjects PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Python shared lib
pybind11_add_module(pygalfunc SHARED $<TARGET_OBJECTS:galfuncobjects>)

if (WIN32)
  set_target_properties(pygalfunc PROPERTIES SUFFIX .dll PREFIX "")
else()
  set_target_properties(pygalfunc PROPERTIES SUFFIX .so PREFIX "")
endif()
target_link_libraries(pygalfunc PUBLIC galfuncobjects)
# Copy the python setup file
add_custom_command(TARGET pygalfunc
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/scripts/setup.py
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# Install the python package so the latest bindings are available
add_custom_command(TARGET pygalfunc
  POST_BUILD
  COMMAND pip3 install -e ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
