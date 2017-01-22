# Archive
add_library(colt-vector include/colt/vector.h src/vector.c)
target_link_libraries(colt-vector PRIVATE colt-misc)
