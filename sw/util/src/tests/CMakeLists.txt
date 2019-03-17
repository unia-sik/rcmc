include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../silver/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../copper/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gold/)

file(GLOB test_files "*.c") 
foreach(file ${test_files})
    get_filename_component(barename ${file} NAME_WE)    
    add_executable(${barename}.elf ${file})
    set_target_properties(${barename}.elf PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/minimal.ld")
    target_link_libraries(${barename}.elf gold silver copper)
endforeach()

