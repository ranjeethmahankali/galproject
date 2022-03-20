include(FetchContent)
find_package(Git REQUIRED)

set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

message("Cloning imnodes into " ${EXTERNAL_DIR}/imnodes)
FetchContent_Declare(imnodes
  GIT_REPOSITORY https://github.com/Nelarius/imnodes.git
  GIT_TAG 8433b51a911ad200482affe2cfd0e07a46de303c
  PREFIX ${EXTERNAL_DIR}
  SOURCE_DIR ${EXTERNAL_DIR}/imnodes
  TMP_DIR ${EXTERNAL_DIR}/temp
  STAMP_DIR ${EXTERNAL_DIR}/stamps
  DOWNLOAD_DIR ${EXTERNAL_DIR}/downloads)

FetchContent_MakeAvailable(imnodes)

add_library(imnodes STATIC ${EXTERNAL_DIR}/imnodes/imnodes.cpp)
target_include_directories(imnodes PUBLIC ${EXTERNAL_DIR}/imnodes)
