cmake_minimum_required(VERSION 3.13)
project(${LUCARIA_TARGET})

# The user/game code is now a library. Keep this target name stable so
# reload_config.hpp can rebuild/reload it without knowing the final exe target.
if(NOT LUCARIA_USER_TARGET)
    set(LUCARIA_USER_TARGET user_game)
endif()

add_subdirectory("${LUCARIA_SOURCE_DIR}/client" "${LUCARIA_BINARY_DIR}/client/${LUCARIA_TARGET_PLATFORM}")

# filter out shaders
set(LUCARIA_GSLC_SOURCES ${LUCARIA_TARGET_SOURCES})
list(FILTER LUCARIA_GSLC_SOURCES INCLUDE REGEX "\.gslc$")
set(LUCARIA_NORMAL_SOURCES ${LUCARIA_TARGET_SOURCES})
list(FILTER LUCARIA_NORMAL_SOURCES EXCLUDE REGEX "\.gslc$")

# User code is static normally, shared when reload is enabled.
# Platforms that cannot cheaply dlopen a game module keep the old static behavior.
set(LUCARIA_USER_LIBRARY_TYPE STATIC)
if(NOT LUCARIA_DISABLE_RELOAD AND NOT ANDROID AND NOT EMSCRIPTEN AND NOT PSP)
    set(LUCARIA_USER_LIBRARY_TYPE SHARED)
endif()

add_library(${LUCARIA_USER_TARGET} ${LUCARIA_USER_LIBRARY_TYPE} ${LUCARIA_NORMAL_SOURCES})

# append shader sources to the user module; shaders belong to user assets/code
foreach(gslc_source IN LISTS LUCARIA_GSLC_SOURCES)
    embed_gslc(${LUCARIA_USER_TARGET} ${gslc_source})
endforeach()

set_target_properties(${LUCARIA_USER_TARGET} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${LUCARIA_USER_TARGET} PRIVATE lucaria_client_core)
target_include_directories(${LUCARIA_USER_TARGET} PRIVATE ${LUCARIA_TARGET_INCLUDES})
target_compile_definitions(${LUCARIA_USER_TARGET} PRIVATE ${LUCARIA_TARGET_DEFINES})
target_compile_options(${LUCARIA_USER_TARGET} PRIVATE ${LUCARIA_TARGET_BUILD_ARGS})

# lucaria_client is now the executable created by the engine CMake.
# Keep the generated binary name equal to the user requested game target.
set_target_properties(lucaria_client PROPERTIES OUTPUT_NAME ${LUCARIA_TARGET})

# In reload mode the executable dlopens the user module, so do not link it.
# In non-reload mode link it normally for a single self-contained executable.
if(NOT LUCARIA_DISABLE_RELOAD AND NOT ANDROID AND NOT EMSCRIPTEN AND NOT PSP)
    add_dependencies(lucaria_client ${LUCARIA_USER_TARGET})
else()
    target_link_libraries(lucaria_client PRIVATE ${LUCARIA_USER_TARGET})
endif()

if(PSP)
    create_pbp_file(
        TARGET lucaria_client
        ICON_PATH NULL
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE ${LUCARIA_TARGET}
        VERSION 01.00)
endif()

# emscripten options
if(EMSCRIPTEN)
    if(NOT LUCARIA_HTML_SHELL)
        set(LUCARIA_HTML_SHELL "${CMAKE_CURRENT_LIST_DIR}/web/shell.html")
    endif()

    set_target_properties(lucaria_client PROPERTIES SUFFIX ".html")
    target_link_options(lucaria_client PRIVATE --shell-file ${LUCARIA_HTML_SHELL})
    target_link_options(lucaria_client PRIVATE -sLLD_REPORT_UNDEFINED)
    target_link_options(lucaria_client PRIVATE -sALLOW_MEMORY_GROWTH=1)
    target_link_options(lucaria_client PRIVATE -sINITIAL_MEMORY=671088640)
    target_link_options(lucaria_client PRIVATE -sMALLOC=emmalloc)
    target_link_options(lucaria_client PRIVATE -sMAX_WEBGL_VERSION=2)
    target_link_options(lucaria_client PRIVATE -sUSE_WEBGL2=1)
    target_link_options(lucaria_client PRIVATE -sFETCH)
    target_link_options(lucaria_client PRIVATE -lopenal)
    
    if(LUCARIA_DEBUG)
        target_link_options(lucaria_client PRIVATE -g)
        target_link_options(lucaria_client PRIVATE -sGL_DEBUG=1)
        target_link_options(lucaria_client PRIVATE -sNO_DISABLE_EXCEPTION_CATCHING)
    else()
        target_link_options(lucaria_client PRIVATE -O3)
        target_link_options(lucaria_client PRIVATE -sGL_DEBUG=0)
    endif()
    
    if(LUCARIA_ASSETS_PACKAGE)
        target_link_options(lucaria_client PRIVATE --preload-file ${LUCARIA_ASSETS_DIR}@/)
    endif()
endif()

# win32 options
if(WIN32 AND LUCARIA_HIDE_CONSOLE)
    target_compile_definitions(lucaria_client PRIVATE -DLUCARIA_HIDE_CONSOLE=1)
    set_target_properties(lucaria_client PROPERTIES WIN32_EXECUTABLE TRUE)
else()
    target_compile_definitions(lucaria_client PRIVATE -DLUCARIA_HIDE_CONSOLE=0)
endif()

# lto optimization
cmake_policy(SET CMP0069 NEW)
function(enable_ipo_recursive TARGET)
    if(NOT TARGET ${TARGET})
        return()
    endif()
    get_property(already_set TARGET ${TARGET} PROPERTY _IPO_MARKER SET)
    if(already_set)
        return()
    endif()
    set_property(TARGET ${TARGET} PROPERTY _IPO_MARKER TRUE)
    set_property(TARGET ${TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    get_target_property(link_libs ${TARGET} LINK_LIBRARIES)
    if(NOT link_libs)
        return()
    endif()
    foreach(lib IN LISTS link_libs)
        if(TARGET ${lib})
            enable_ipo_recursive(${lib})
        endif()
    endforeach()
endfunction()

if(NOT LUCARIA_DEBUG)
    enable_ipo_recursive(lucaria_client)
    enable_ipo_recursive(${LUCARIA_USER_TARGET})
endif()
