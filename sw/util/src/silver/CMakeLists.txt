include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(src_files
    "fgmp.s"
    "fgmp_block.s"
    "fgmp_block.c"
)

add_library(silver ${src_files})
