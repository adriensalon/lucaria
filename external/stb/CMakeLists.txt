set(lucaria_external_stb_sources "upstream/stb/stb_image.cpp")
add_library(lucaria_external_stb STATIC EXCLUDE_FROM_ALL ${lucaria_external_stb_sources})
set_target_properties(lucaria_external_stb PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_stb PUBLIC upstream/stb)
