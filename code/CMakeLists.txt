cmake_minimum_required(VERSION 3.18)

project(EDA093-lab1 LANGUAGES C)

add_executable(lsh
    lsh.c
    parse.c
)

# header search
target_include_directories(lsh PRIVATE .)
# link lib
target_link_libraries(lsh PRIVATE readline termcap)
# compile options
target_compile_options(lsh PRIVATE "-ggdb3" "-O0" "-Wall" "-Wextra")

# output dir
set_target_properties(lsh PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# clean all
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory
${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove
${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMENT "clean all build files"
)

# build info
message(STATUS "${PROJECT_NAME}")
message(STATUS "exec output: ${CMAKE_BINARY_DIR}/bin")
