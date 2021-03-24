cmake_minimum_required(VERSION 3.16.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VCPKG_TARGET_TRIPLET x64-linux)
set(CMAKE_PREFIX_PATH $ENV{VCPKG_PATH}/installed/${VCPKG_TARGET_TRIPLET})
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake)

# GALCORE
project(galcore)

find_package(Boost REQUIRED)
if (Boost_FOUND)
    add_compile_definitions(BOOST_UUID_FORCE_AUTO_LINK)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(glm CONFIG REQUIRED)

include_directories(include)

file(GLOB GALCORE_SRC "src/galcore/*.cpp")
add_library(galcore STATIC ${GALCORE_SRC})

target_link_libraries(galcore PUBLIC glm::glm)

# GALVIEW
project(galview)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui CONFIG REQUIRED)

file(GLOB GALVIEW_SRC "src/galview/*.cpp")
add_executable(galview ${GALVIEW_SRC})

target_link_libraries(galview PRIVATE
  galcore
  imgui::imgui
  GLEW::GLEW
  glfw)

# Copy the shaders to bin dir
add_custom_command(TARGET galview
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/shaders/*.glsl ${PROJECT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/fonts/*.* ${PROJECT_BINARY_DIR}
)

# Tests
