set(ARCHIVE_NAME Watchdog)
set(ARCHIVE_PATH deps/${ARCHIVE_NAME})

#####
# Dependencies
###
set(DEPENDENCIES "Alligator" "Chain")

#####
# Archive
###
file(GLOB HEADER_FILES ${ARCHIVE_PATH}/*.h)
file(GLOB SOURCE_FILES ${ARCHIVE_PATH}/*.c)
add_library(${ARCHIVE_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(${ARCHIVE_NAME} PRIVATE ${DEPENDENCIES})

message("${ARCHIVE_NAME}: ${HEADER_FILES} ${SOURCE_FILES} included")
