## Minimal cmake version
cmake_minimum_required(VERSION 2.8)

## Project name
project(baremetal_util C ASM)
#If a source file is not detected, try to 'touch' this file

## Options
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/)

set(CMAKE_C_FLAGS "${TARGET_ARCH} ${TARGET_BUILD_OPTIONS}")
set(CMAKE_ASM_FLAGS "${TARGET_ARCH}")

add_subdirectory(src/copper)
add_subdirectory(src/silver)
add_subdirectory(src/gold)
add_subdirectory(src/tests)
 
