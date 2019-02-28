## Minimal cmake version
cmake_minimum_required(VERSION 2.8)

## Project name
project(one_to_one C ASM)
#If a source file is not detected, try to 'touch' this file

## Options 
set (CMAKE_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set (TOOLS ${RCMC_ROOT_DIR}/tools/)

## Targets
set(src_vhd
    "${CMAKE_CURRENT_SOURCE_DIR}/src/constants.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/LibNode.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libproc.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/libeu.vhd"
 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cpu_top.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/csrfile.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/d_stage.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dmem_content.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dmem_replacement.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/e_stage.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/eu.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/f_stage.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/m_stage.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NetworkInterfaceSend.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NetworkInterfaceRecv.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/flit_buffer.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/flit_buffer2.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/top_level.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nocunit.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node_double_corner.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node_sort.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node_empty.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node_no_rdy.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node_srr.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node_drr.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pnoo_node_cg.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/regfile_ram_replacement.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/regfile.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/w_stage.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clock.vhd"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/reset.vhd"
)

set (MAKE_GHDL "ghdl" "-a" "--workdir=${CMAKE_PROJECT_SOURCE_DIR}/build/ghdl")
set (MAKE_GHDL_TOP "ghdl" "-e" "--workdir=${CMAKE_PROJECT_SOURCE_DIR}/build/ghdl" "top_level")
set (MAKE_GHDL_DOC "ghdl" "--xref-html" "--format=css")
set (GHDL_CMD ${MAKE_GHDL} ${src_vhd})
set (GHDL_CMD_DOC ${MAKE_GHDL_DOC} ${src_vhd})
set (GHDL_CMD_DOC_SYNC "rsync" "-a" "html" "${CMAKE_PROJECT_SOURCE_DIR}")

add_subdirectory(tests/)

if (EXISTS ${CUSTOM_ELF_FILE})
    add_custom_target(custom_mem_content ALL
        COMMAND ${TOOLS}/shexdump/shexdump -V ${CUSTOM_ELF_FILE} 1 65535 > ${CMAKE_PROJECT_SOURCE_DIR}/src/dmem_content.vhd
    )

    add_custom_target(custom_elf ALL
        COMMAND ${GHDL_CMD}
        COMMAND ${MAKE_GHDL_TOP}
        COMMAND ${GHDL_CMD_DOC}
        COMMAND ${GHDL_CMD_DOC_SYNC}
        DEPENDS custom_mem_content
    )  
endif()
