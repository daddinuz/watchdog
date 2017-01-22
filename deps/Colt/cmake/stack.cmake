# Archives
add_library(colt-stack include/colt/stack.h src/stack.c)
target_link_libraries(colt-stack PRIVATE colt-misc colt-list colt-xor-list colt-forward-list colt-vector)
