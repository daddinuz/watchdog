cmake_minimum_required(VERSION 3.0)
project(Watchdog)

set(CMAKE_C_FLAGS "-std=c99 -Werror -Wall -Wextra -Wpedantic -Wuninitialized -Wmaybe-uninitialized -Wno-unused-function")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
add_subdirectory(deps/Chain)
include_directories(include/)

#####
# Archive
###
set(PROJECT_PATH "${PROJECT_SOURCE_DIR}")
set(INCLUDE_PATH "${PROJECT_PATH}/include/Watchdog")
set(SOURCE_PATH "${PROJECT_PATH}/src")
file(GLOB SOURCE_FILES "${SOURCE_PATH}/*.c")

add_library(watchdog "${SOURCE_FILES}")
target_include_directories(watchdog PRIVATE deps/Chain/include)
target_link_libraries(watchdog PRIVATE chain)

#####
# Examples
###
set(EXAMPLE_PATH "${PROJECT_PATH}/examples")

add_executable(example "${EXAMPLE_PATH}/example.c")
target_compile_options(example PRIVATE -Wno-unused-variable)
target_link_libraries(example watchdog)
