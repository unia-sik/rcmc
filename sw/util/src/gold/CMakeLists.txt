include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../copper/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../silver/)
file(GLOB src_files "*.c") 

add_library(gold ${src_files})

