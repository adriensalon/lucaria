if(LUCARIA_BUILD_COMPILERS)

    if(WIN32)
        set(nfd_sources "upstream/nfd/src/nfd_win.cpp")    
    elseif(UNIX)
        set(nfd_sources "upstream/nfd/src/nfd_portal.cpp")
    endif()

    add_library(lucaria_external_nfd STATIC EXCLUDE_FROM_ALL ${nfd_sources})
    target_include_directories(lucaria_external_nfd PUBLIC "upstream/nfd/src/include")
    target_include_directories(lucaria_external_nfd PRIVATE "upstream/nfd/src")
    target_compile_features(lucaria_external_nfd PUBLIC 
		c_std_99 
		cxx_std_17)

    if(WIN32)
        target_compile_definitions(lucaria_external_nfd PRIVATE 
			NOMINMAX 
			WIN32_LEAN_AND_MEAN)
        target_link_libraries(lucaria_external_nfd PUBLIC 
			ole32 
			uuid 
			shell32)
    elseif(UNIX)
        target_link_libraries(lucaria_external_nfd PUBLIC pthread)
    endif()

    if(MSVC)
        target_compile_options(lucaria_external_nfd PRIVATE /wd4996)
    endif()

endif()
