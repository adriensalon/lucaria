if(LUCARIA_BUILD_COMPILERS)

	add_library(lucaria_external_ppqsort INTERFACE)
	target_include_directories(lucaria_external_ppqsort INTERFACE "upstream/ppqsort/include")
	target_compile_features(lucaria_external_ppqsort INTERFACE cxx_std_17)

endif()
