if(LUCARIA_BUILD_COMPILERS)

    add_library(lucaria_external_pugixml STATIC EXCLUDE_FROM_ALL "upstream/pugixml/src/pugixml.cpp")
    target_include_directories(lucaria_external_pugixml PUBLIC "upstream/pugixml/src")
    target_compile_features(lucaria_external_pugixml PUBLIC cxx_std_11)

    if(MSVC)
        target_compile_options(lucaria_external_pugixml PRIVATE
            /wd4996)
    endif()

endif()
