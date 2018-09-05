unset(WATCHDOG_FORCE_OVERRIDE CACHE)

set(ARCHIVE_NAME watchdog)
message("${ARCHIVE_NAME}@${CMAKE_CURRENT_LIST_DIR} using: ${CMAKE_CURRENT_LIST_FILE}")

file(GLOB ARCHIVE_HEADERS ${CMAKE_CURRENT_LIST_DIR}/*.h)
file(GLOB ARCHIVE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.c)
add_library(${ARCHIVE_NAME} ${ARCHIVE_HEADERS} ${ARCHIVE_SOURCES})
target_link_libraries(${ARCHIVE_NAME} PRIVATE panic process)

# Optional features
option(WATCHDOG_FORCE_OVERRIDE "Force standard library allocators overriding" OFF)

if (WATCHDOG_FORCE_OVERRIDE)
    target_compile_definitions(${ARCHIVE_NAME} PUBLIC WATCHDOG_FORCE_OVERRIDE=1)
else ()
    target_compile_definitions(${ARCHIVE_NAME} PUBLIC WATCHDOG_FORCE_OVERRIDE=0)
endif (WATCHDOG_FORCE_OVERRIDE)
