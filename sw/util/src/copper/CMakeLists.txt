include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(src_files
    "crt.S"
    "syscalls.c"
    "random.c"
)

add_library(copper ${src_files})
set_target_properties(copper PROPERTIES LINK_FLAGS "-T minimal.ld")
