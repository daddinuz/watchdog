# Archive
add_library(colt-queue include/colt/queue.h src/queue.c)
target_link_libraries(colt-queue PRIVATE colt-misc colt-list colt-xor-list colt-forward-list colt-vector)
