file(READ "upstream/vorbis/configure.ac" CONFIGURE_AC_CONTENTS)
string(REGEX MATCH "AC_INIT\\(\\[libvorbis\\],\\[([0-9]*).([0-9]*).([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

function(get_version_info result current_var_name age_var_name revision_var_name)
    string(REGEX MATCH "${current_var_name}=([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
    set(VERSION_INFO_CURRENT ${CMAKE_MATCH_1})

    string(REGEX MATCH "${age_var_name}=([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
    set(VERSION_INFO_AGE ${CMAKE_MATCH_1})

    string(REGEX MATCH "${revision_var_name}=([0-9]*)" DUMMY ${CONFIGURE_AC_CONTENTS})
    set(VERSION_INFO_REVISION ${CMAKE_MATCH_1})

    math(EXPR VERSION_INFO_CURRENT_MINUS_AGE "${VERSION_INFO_CURRENT} - ${VERSION_INFO_AGE}")

    set(${result} "${VERSION_INFO_CURRENT_MINUS_AGE}.${VERSION_INFO_AGE}.${VERSION_INFO_REVISION}" PARENT_SCOPE)
endfunction()

set(vorbis_lib_sources
    "upstream/vorbis/lib/mdct.c"
    "upstream/vorbis/lib/smallft.c"
    "upstream/vorbis/lib/block.c"
    "upstream/vorbis/lib/envelope.c"
    "upstream/vorbis/lib/window.c"
    "upstream/vorbis/lib/lsp.c"
    "upstream/vorbis/lib/lpc.c"
    "upstream/vorbis/lib/analysis.c"
    "upstream/vorbis/lib/synthesis.c"
    "upstream/vorbis/lib/psy.c"
    "upstream/vorbis/lib/info.c"
    "upstream/vorbis/lib/floor1.c"
    "upstream/vorbis/lib/floor0.c"
    "upstream/vorbis/lib/res0.c"
    "upstream/vorbis/lib/mapping0.c"
    "upstream/vorbis/lib/registry.c"
    "upstream/vorbis/lib/codebook.c"
    "upstream/vorbis/lib/sharedbook.c"
    "upstream/vorbis/lib/lookup.c"
    "upstream/vorbis/lib/bitrate.c")

set(vorbis_libfile_sources
    "upstream/vorbis/lib/vorbisfile.c")

set(vorbis_libenc_sources
    "upstream/vorbis/lib/vorbisenc.c")

set(vorbis_sources
    ${vorbis_lib_sources}
    ${vorbis_libfile_sources}
    ${vorbis_libenc_sources})

add_library(lucaria_external_vorbis STATIC ${vorbis_sources})
target_include_directories(lucaria_external_vorbis PRIVATE "upstream/vorbis")
target_include_directories(lucaria_external_vorbis PRIVATE "upstream/vorbis/lib")
target_include_directories(lucaria_external_vorbis PUBLIC "upstream/vorbis/include")
target_link_libraries(lucaria_external_vorbis PUBLIC lucaria_external_ogg)

get_version_info(VORBIS_VERSION_INFO "V_LIB_CURRENT" "V_LIB_AGE" "V_LIB_REVISION")
set_target_properties(lucaria_external_vorbis PROPERTIES SOVERSION ${VORBIS_VERSION_INFO})
get_version_info(VORBISENC_VERSION_INFO "VE_LIB_CURRENT" "VE_LIB_AGE" "VE_LIB_REVISION")
set_target_properties(lucaria_external_vorbis PROPERTIES SOVERSION ${VORBISENC_VERSION_INFO})
get_version_info(VORBISFILE_VERSION_INFO "VF_LIB_CURRENT" "VF_LIB_AGE" "VF_LIB_REVISION")
set_target_properties(lucaria_external_vorbis PROPERTIES SOVERSION ${VORBISFILE_VERSION_INFO})

if(MSVC)
    target_compile_options(lucaria_external_vorbis PRIVATE /wd4244)
    target_compile_options(lucaria_external_vorbis PRIVATE /wd4305)
    target_compile_options(lucaria_external_vorbis PRIVATE /wd4267)
    target_compile_options(lucaria_external_vorbis PRIVATE /wd4996)
endif()
