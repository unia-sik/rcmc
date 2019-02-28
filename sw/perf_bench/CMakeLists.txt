## Minimal cmake version
cmake_minimum_required(VERSION 2.8)

## Project name
project(perf_bench C ASM)
#If a source file is not detected, try to 'touch' this file

## Options
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(CMAKE_C_FLAGS "${TARGET_ARCH} ${TARGET_BUILD_OPTIONS}")
set(CMAKE_ASM_FLAGS "${TARGET_ARCH}")

## Paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${RCMC_ROOT_DIR}/sw/util/src/copper/)
include_directories(${RCMC_ROOT_DIR}/sw/util/src/silver/)
include_directories(${RCMC_ROOT_DIR}/sw/util/src/gold/)
link_directories(${RCMC_ROOT_DIR}/sw/util/lib)

## Targets
file(GLOB files 
    "src/benchmark/app/*.c"
    "src/benchmark/misc/*.c"
    "src/benchmark/mpi/*.c"
    "src/benchmark/sort/*.c"
)
foreach(file ${files})
    get_filename_component(barename ${file} NAME_WE)    
    add_executable(${barename}.elf ${src_test_suite} ${file})
    set_target_properties(${barename}.elf PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/src/minimal.ld")
    target_link_libraries(${barename}.elf ${RCMC_ROOT_DIR}/sw/util/lib/libgold.a ${RCMC_ROOT_DIR}/sw/util/lib/libsilver.a ${RCMC_ROOT_DIR}/sw/util/lib/libcopper.a m)
endforeach()



