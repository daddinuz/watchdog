cmake_minimum_required(VERSION 3.10)
project(watchdog C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

include_directories(deps)
include(deps/panic/panic.cmake)

add_library(watchdog sources/watchdog.h sources/watchdog.c)
target_link_libraries(watchdog PRIVATE panic)

add_executable(main examples/main.c)
target_link_libraries(main PRIVATE watchdog)
target_include_directories(main PRIVATE sources)
