cmake_minimum_required(VERSION 3.10)

project(libogg)

# Specify the source files
set(OGG_SRCS
    "src/bitwise.c"
    "src/framing.c"
)

# Specify the header files
set(OGG_HDRS
    "include/ogg/config_types.h"
    "include/ogg/ogg.h"
    "include/ogg/os_types.h"
)

# Create the library
add_library(ogg STATIC ${OGG_SRCS})

# Specify include directories
target_include_directories(ogg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Export the target
add_library(Ogg::ogg ALIAS ogg)
