if(LUCARIA_BUILD_COMPILERS)

	add_library(lucaria_external_json INTERFACE)
	target_include_directories(lucaria_external_json INTERFACE "upstream/json/single_include")
	target_compile_features(lucaria_external_json INTERFACE cxx_std_11)

endif()
