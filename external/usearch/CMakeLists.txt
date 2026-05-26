if(LUCARIA_BUILD_COMPILERS)

    set(USEARCH_ROOT "upstream/usearch")
    set(NUMKONG_ROOT "upstream/numkong")
    set(STRINGZILLA_ROOT "upstream/stringzilla")
    add_library(lucaria_external_usearch INTERFACE)
    target_include_directories(lucaria_external_usearch INTERFACE
        "upstream/usearch/include"
        "upstream/numkong/include"
        "upstream/stringzilla/include")

    target_compile_features(lucaria_external_usearch INTERFACE cxx_std_17)

endif()