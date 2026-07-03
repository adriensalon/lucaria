set(lucaria_external_stb_sources "source/stb_image.cpp")
add_library(lucaria_external_stb STATIC EXCLUDE_FROM_ALL ${lucaria_external_stb_sources})
set_target_properties(lucaria_external_stb PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_stb PUBLIC upstream/stb)

if(LUCARIA_COMPILERS)
	target_sources(lucaria_external_stb_sources PRIVATE "source/stb_image_resize2.cpp")
endif()
