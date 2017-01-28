cmake_minimum_required(VERSION 3.0)
project(Chain)

set(CMAKE_C_FLAGS "-std=c99 -Werror -Wall -Wextra -Wpedantic -Wuninitialized -Wmaybe-uninitialized")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
include_directories(include/)

#####
# Archive
###
set(PROJECT_PATH "${PROJECT_SOURCE_DIR}")
set(INCLUDE_PATH "${PROJECT_PATH}/include/Chain")
set(SOURCE_PATH "${PROJECT_PATH}/src")
file(GLOB SOURCE_FILES "${SOURCE_PATH}/*.c")

add_library(chain "${SOURCE_FILES}")
