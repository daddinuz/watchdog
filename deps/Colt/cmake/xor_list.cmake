# Archive
add_library(colt-xor-list include/colt/xor_list.h src/xor_list.c)
target_link_libraries(colt-xor-list PRIVATE colt-misc colt-pair)
