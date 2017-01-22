# Archive
add_library(colt-pair include/colt/pair.h src/pair.c)
target_link_libraries(colt-pair PRIVATE colt-misc)
