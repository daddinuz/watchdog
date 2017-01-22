cmake_minimum_required(VERSION 3.0)
project(Colt)

set(CMAKE_C_FLAGS "-std=c99 -Werror -Wall -Wextra -Wpedantic -Wuninitialized -Wmaybe-uninitialized")
set(CMAKE_CXX_FLAGS "-std=c++11 -Werror -Wall -Wextra -Wpedantic -Wuninitialized -Wmaybe-uninitialized")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
include_directories(include/)

# Common
add_library(colt-misc src/internal.h src/internal.c src/allocator.h src/allocator.c)

# Stable
include(cmake/pair.cmake)
#include(cmake/list.cmake)
include(cmake/xor_list.cmake)
#include(cmake/forward_list.cmake)
#include(cmake/vector.cmake)
#include(cmake/queue.cmake)
#include(cmake/stack.cmake)

# Archive
#add_library(colt include/colt/colt.h)
#set_target_properties(colt PROPERTIES LINKER_LANGUAGE C)
#target_link_libraries(colt PRIVATE colt-misc colt-pair colt-list colt-xor-list colt-forward-list colt-vector colt-queue colt-stack)
