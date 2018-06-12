add_library(watchdog ${CMAKE_CURRENT_LIST_DIR}/watchdog.h ${CMAKE_CURRENT_LIST_DIR}/watchdog.c)
target_link_libraries(watchdog PRIVATE panic process)
