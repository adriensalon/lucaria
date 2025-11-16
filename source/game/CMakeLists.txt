cmake_minimum_required(VERSION 3.13)
project(${LUCARIA_TARGET})

add_subdirectory("${LUCARIA_SOURCE_DIR}/client" "${LUCARIA_BINARY_DIR}/client/${LUCARIA_TARGET_PLATFORM}")

add_executable(${LUCARIA_TARGET} ${LUCARIA_TARGET_SOURCES})
set_target_properties(${LUCARIA_TARGET} PROPERTIES CXX_STANDARD 17)
target_link_libraries(${LUCARIA_TARGET} PRIVATE lucaria_client)

target_include_directories(${LUCARIA_TARGET} PRIVATE ${LUCARIA_TARGET_INCLUDES})
target_compile_definitions(${LUCARIA_TARGET} PRIVATE ${LUCARIA_TARGET_DEFINES})
target_compile_options(${LUCARIA_TARGET} PRIVATE ${LUCARIA_TARGET_BUILD_ARGS})

if(NOT LUCARIA_HTML_SHELL)
    set(LUCARIA_HTML_SHELL "${CMAKE_CURRENT_LIST_DIR}/shell.html")
endif()

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    
    if(LUCARIA_DEBUG)
        message("-- Configuring emscripten build options for Debug")
        set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "\
            -g \
            --shell-file ${LUCARIA_HTML_SHELL} \
            -s LLD_REPORT_UNDEFINED \
            -s ALLOW_MEMORY_GROWTH=1\
            -s INITIAL_MEMORY=671088640 \
            -s MALLOC=emmalloc\
            -s MAX_WEBGL_VERSION=2 \
            -s USE_WEBGL2=1 \
            -s GL_DEBUG=1 \
            -s FETCH \
            -s NO_DISABLE_EXCEPTION_CATCHING \
            -l openal")
    else()
        message("-- Configuring emscripten build options for Release")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        	-O3 \
            --shell-file ${LUCARIA_HTML_SHELL} \
        	-s LLD_REPORT_UNDEFINED \
        	-s ALLOW_MEMORY_GROWTH=1 \
            -s INITIAL_MEMORY=671088640 \
            -s MALLOC=emmalloc\
        	-s MAX_WEBGL_VERSION=2 \
        	-s USE_WEBGL2=1 \
            -s GL_DEBUG=0 \
        	-s FETCH \
        	-l openal")
    endif()
endif()
