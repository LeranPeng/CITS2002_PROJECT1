cmake_minimum_required(VERSION 3.28)
project(MLTOC VERSION 0.1.0 LANGUAGES C)

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")  
set(CMAKE_C_STANDARD 11)

# Recursively find all .c files in the src directory
file(GLOB_RECURSE SRC_FILES "src/*.c")

# Recursively find all .h files in the includes directory
file(GLOB_RECURSE HEADER_FILES "includes/*.h")

# Add the executable file
add_executable(MLTOC ${HEADER_FILES} ${SRC_FILES})

include(CTest)
enable_testing()