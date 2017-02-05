cmake_minimum_required(VERSION 3.0)
project(Chain)

include_directories(include ${ALLIGATOR_PATH}/include)

#####
# Archive
###
set(HEADER_PATH ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME})
set(SOURCE_PATH ${PROJECT_SOURCE_DIR}/src)
file(GLOB HEADER_FILES ${HEADER_PATH}/*.h)
file(GLOB SOURCE_FILES ${SOURCE_PATH}/*.c)
add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE Alligator)
