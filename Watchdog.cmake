cmake_minimum_required(VERSION 3.0)
project(Watchdog)

#####
# Dependencies
###
set(DEPENDENCY_PATH ../)
include("${DEPENDENCY_PATH}/Alligator/Alligator.cmake")
include("${DEPENDENCY_PATH}/Chain/Chain.cmake")
include_directories(${DEPENDENCY_PATH})

#####
# Archive
###
file(GLOB HEADER_FILES ./*.h)
file(GLOB SOURCE_FILES ./*.c)
add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE Alligator Chain)
