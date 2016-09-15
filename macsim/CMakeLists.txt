project("MacSim")
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-O2 -Wall")
set(CLANG true)

include_directories(src)
include_directories(src/arch)
include_directories(src/noc)

set(SOURCE_FILES macsim-seq.c)
add_subdirectory(src)
add_subdirectory(src/arch)
add_subdirectory(src/noc)

add_executable(macsim-seq ${SOURCE_FILES})
target_link_libraries(macsim-seq Src Arch Noc m)

install(TARGETS macsim-seq DESTINATION bin)

