## Options
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/)
set(CMAKE_C_FLAGS "${TARGET_ARCH} ${TARGET_BUILD_OPTIONS}")
set(CMAKE_ASM_FLAGS "${TARGET_ARCH}")


## Paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${RCMC_ROOT_DIR}/sw/util/src/silver)
link_directories(${RCMC_ROOT_DIR}/sw/util/lib)


## Targets
set(src_test_suite "${CMAKE_CURRENT_SOURCE_DIR}/src/test_suite.s")

file(GLOB files "src/tests/*.c")
foreach(file ${files})
    get_filename_component(barename ${file} NAME_WE)    
    add_executable(${barename}.elf ${file} ${src_test_suite})
    set_target_properties(${barename}.elf PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/src/minimal.ld")
    target_link_libraries(${barename}.elf silver)
        
    add_custom_target(${barename}.vhd ALL
        COMMAND ${TOOLS}/shexdump/shexdump -V ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${barename}.elf 1 65535 > ${CMAKE_PROJECT_SOURCE_DIR}/src/dmem_content.vhd
        DEPENDS ${barename}.elf
    )

    add_custom_target(${barename} ALL
        COMMAND ${GHDL_CMD}
        COMMAND ${MAKE_GHDL_TOP}
        COMMAND ${GHDL_CMD_DOC}
        COMMAND ${GHDL_CMD_DOC_SYNC}
        DEPENDS ${barename}.vhd
    )  
endforeach()

file(GLOB files "${CMAKE_PROJECT_SOURCE_DIR}/build/ghdl/*")
foreach(file ${files})
    set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${file}")
endforeach()
