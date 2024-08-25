cmake_minimum_required(VERSION 3.28)
project(MLTOC VERSION 0.1.0 LANGUAGES C)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")  

aux_source_directory(. DIR_SRCS)

add_executable(MLTOC ${DIR_SRCS})

include(CTest)
enable_testing()