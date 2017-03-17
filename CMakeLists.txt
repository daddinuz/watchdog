cmake_minimum_required(VERSION 3.0)
project(Watchdog)

set(CMAKE_C_FLAGS "-std=c99 -Werror -Wall -Wextra -Wpedantic -Wuninitialized -Wmaybe-uninitialized -Wno-unused-function")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
include_directories(include/${PROJECT_NAME})

#####
# Dependencies
###
set(DEPENDENCY_PATH ${PROJECT_SOURCE_DIR}/deps)

include_directories(${DEPENDENCY_PATH}/Alligator)
include(${DEPENDENCY_PATH}/Alligator/Alligator.cmake)

include_directories(${DEPENDENCY_PATH}/Chain)
include(${DEPENDENCY_PATH}/Chain/Chain.cmake)

#####
# Archive
###
set(HEADER_PATH ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME})
set(SOURCE_PATH ${PROJECT_SOURCE_DIR}/src)
file(GLOB HEADER_FILES ${HEADER_PATH}/*.h)
file(GLOB SOURCE_FILES ${SOURCE_PATH}/*.c)
add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE Alligator Chain)

#####
# Examples
###
set(EXAMPLE_PATH ${PROJECT_SOURCE_DIR}/examples)
add_executable(example ${EXAMPLE_PATH}/example.c)
target_link_libraries(example PRIVATE ${PROJECT_NAME})
target_compile_options(example PRIVATE -Wno-unused-variable)
