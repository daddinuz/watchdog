# Archive
add_library(colt-list include/colt/list.h src/list.c)
target_link_libraries(colt-list PRIVATE colt-misc)
