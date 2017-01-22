# Archive
add_library(colt-forward-list include/colt/forward_list.h src/forward_list.c)
target_link_libraries(colt-forward-list PRIVATE colt-misc)
