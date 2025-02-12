cmake_minimum_required(VERSION 3.20.0)

file(GLOB GALVIEW_SRC "*.cpp")
add_executable(galview ${GALVIEW_SRC})

target_link_libraries(galview PRIVATE
  galcore
  galfunc
  imgui::imgui
  GLEW::GLEW
  glfw
  freetype
)

target_include_directories(galview PRIVATE
  "include/")

if (WIN32)
  set_property(TARGET galview PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

file(GLOB SHADER_FILES ${PROJECT_SOURCE_DIR}/shaders/*.glsl)
file(GLOB FONT_FILES ${PROJECT_SOURCE_DIR}/fonts/*.*)

# Copy the shaders to bin dir
add_custom_command(TARGET galview
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${SHADER_FILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  COMMAND ${CMAKE_COMMAND} -E copy ${FONT_FILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
