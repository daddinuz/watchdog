cmake_minimum_required(VERSION 3.8)
project(watchdog C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

# dependencies
include_directories(deps)
include(deps/panic/build.cmake)

# archive
include_directories(sources)
include(sources/build.cmake)

# examples
include(examples/build.cmake)
