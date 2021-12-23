if(NOT DEFINED ENV{VCPKG_PATH})
  message(FATAL_ERROR "Please clone vcpkg and set the VCPKG_PATH environment variable")
endif()

set(VCPKG_FEATURE_FLAGS versions)
set(VCPKG_TARGET_TRIPLET x64-linux)
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake)
