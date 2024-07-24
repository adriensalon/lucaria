cmake_minimum_required(VERSION 3.13)
project(lucaria)

function(find_program_recursive OUTPUT_VARIABLE DIRECTORY EXECUTABLE_NAME)
    file(GLOB_RECURSE EXECUTABLE_PATHS "${DIRECTORY}/*${EXECUTABLE_NAME}*")

    foreach(EXECUTABLE_PATH ${EXECUTABLE_PATHS})
        # Get the filename without extension and the extension
        get_filename_component(NAME_WITHOUT_EXT ${EXECUTABLE_PATH} NAME_WLE)
        get_filename_component(FILE_EXT ${EXECUTABLE_PATH} EXT)

        if(NAME_WITHOUT_EXT STREQUAL "${EXECUTABLE_NAME}")
            if(WIN32)
                # For Windows executables, check extensions
                if(FILE_EXT STREQUAL ".exe" OR FILE_EXT STREQUAL ".bat")
                    set(${OUTPUT_VARIABLE} ${EXECUTABLE_PATH} PARENT_SCOPE)
                    break()
                endif()
            elseif(UNIX)
                # For Unix-like systems, check if the file is executable
                if(NOT FILE_EXT)
                    execute_process(COMMAND ${CMAKE_COMMAND} -E test -x ${EXECUTABLE_PATH}
                                    RESULT_VARIABLE IS_EXECUTABLE)
                    if(IS_EXECUTABLE EQUAL 0)
                        set(${OUTPUT_VARIABLE} ${EXECUTABLE_PATH} PARENT_SCOPE)
                        break()
                    endif()
                endif()
            endif()
        endif()
    endforeach()
endfunction()


# find emsdk
if(DEFINED ENV{EMSDK})
    set(EMSDK $ENV{EMSDK})
elseif(DEFINED ENV{EMSCRIPTEN})
    set(EMSDK $ENV{EMSCRIPTEN})
endif()

if(EMSDK AND EXISTS "${EMSDK}")
    message(STATUS "Found Emsdk at ${EMSDK}")
    set(EMSDK_FOUND TRUE)
else()
    find_program(EMCC_EXECUTABLE NAMES emcc PATHS ENV PATH)

    if(EMCC_EXECUTABLE)
        get_filename_component(EMSDK_DIR "${EMCC_EXECUTABLE}" DIRECTORY)
        get_filename_component(EMSDK "${EMSDK_DIR}/../../" ABSOLUTE)

        if(EXISTS "${EMSDK}")
            message(STATUS "Found Emscripten SDK using emcc at ${EMSDK}")
            set(EMSDK_FOUND TRUE)
        endif()
    endif()
endif()

if(EMSDK_FOUND)
    set(EMSDK_TOOLCHAIN "${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
    message(STATUS "Found Emscripten toolchain file at ${EMSDK_TOOLCHAIN}\n")
else()
    message(FATAL_ERROR "Emscripten SDK could not be located")
endif()

# compiler
set(COMPILER_BUILD_DIR ${CMAKE_BINARY_DIR}/compiler)
set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/install)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${COMPILER_BUILD_DIR}
    COMMAND ${CMAKE_COMMAND}
    -S ${CMAKE_CURRENT_LIST_DIR}/compiler
    -B ${COMPILER_BUILD_DIR}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    COMMAND ${CMAKE_COMMAND} --build ${COMPILER_BUILD_DIR})
message(" ")
find_program_recursive(COMPILER_EXECUTABLE "${COMPILER_BUILD_DIR}" "lucaria_compiler")
find_program_recursive(ETCPAK_EXECUTABLE "${COMPILER_BUILD_DIR}" "etcpak")
find_program_recursive(GLTF2OZZ_EXECUTABLE "${COMPILER_BUILD_DIR}" "gltf2ozz")
find_program_recursive(OGGENC_EXECUTABLE "${COMPILER_BUILD_DIR}" "oggenc")
message(STATUS "Found compiler at ${COMPILER_EXECUTABLE}")
message(STATUS "Found etcpak at ${ETCPAK_EXECUTABLE}")
message(STATUS "Found gltf2ozz at ${GLTF2OZZ_EXECUTABLE}")
message(STATUS "Found oggenc at ${OGGENC_EXECUTABLE}")
get_filename_component(COMPILER_NAME ${COMPILER_EXECUTABLE} NAME)
get_filename_component(ETCPAK_NAME ${ETCPAK_EXECUTABLE} NAME)
get_filename_component(GLTF2OZZ_NAME ${GLTF2OZZ_EXECUTABLE} NAME)
get_filename_component(OGGENC_NAME ${OGGENC_EXECUTABLE} NAME)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/compiler
    COMMAND ${CMAKE_COMMAND} -E rename ${COMPILER_EXECUTABLE} ${INSTALL_DIR}/compiler/${COMPILER_NAME}
    COMMAND ${CMAKE_COMMAND} -E rename ${ETCPAK_EXECUTABLE} ${INSTALL_DIR}/compiler/${ETCPAK_NAME}
    COMMAND ${CMAKE_COMMAND} -E rename ${GLTF2OZZ_EXECUTABLE} ${INSTALL_DIR}/compiler/${GLTF2OZZ_NAME}
    COMMAND ${CMAKE_COMMAND} -E rename ${OGGENC_EXECUTABLE} ${INSTALL_DIR}/compiler/${OGGENC_NAME})
set(COMPILER_EXECUTABLE ${INSTALL_DIR}/compiler/${COMPILER_NAME})
set(ETCPAK_EXECUTABLE ${INSTALL_DIR}/compiler/${ETCPAK_NAME})
set(GLTF2OZZ_EXECUTABLE ${INSTALL_DIR}/compiler/${GLTF2OZZ_NAME})
set(OGGENC_EXECUTABLE ${INSTALL_DIR}/compiler/${OGGENC_NAME})
message(" ")

# assets
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/client
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INSTALL_DIR}/client/assets
    COMMAND ${COMPILER_EXECUTABLE}
        -i ${CMAKE_CURRENT_LIST_DIR}/assets
        -o ${INSTALL_DIR}/client/assets
        -etcpak ${ETCPAK_EXECUTABLE}
        -gltf2ozz ${GLTF2OZZ_EXECUTABLE}
        -oggenc ${OGGENC_EXECUTABLE}
    WORKING_DIRECTORY ${INSTALL_DIR}
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_output
    ERROR_VARIABLE configure_error)
message("${configure_output}")

# client
set(CLIENT_BUILD_DIR ${CMAKE_BINARY_DIR}/client)
# execute_process(
#     COMMAND ${CMAKE_COMMAND}
#     -S ${CMAKE_CURRENT_LIST_DIR}/client
#     -B ${CLIENT_BUILD_DIR}
#     -DCMAKE_TOOLCHAIN_FILE=${EMSDK_TOOLCHAIN}
#     -G Ninja)
add_custom_command(
    OUTPUT ${CLIENT_BUILD_DIR}/lucaria.html
    COMMAND ${CMAKE_COMMAND}
        -S ${CMAKE_CURRENT_LIST_DIR}/client
        -B ${CLIENT_BUILD_DIR}
        -DCMAKE_TOOLCHAIN_FILE=${EMSDK_TOOLCHAIN}
        -G Ninja
    COMMAND ${CMAKE_COMMAND} --build ${CLIENT_BUILD_DIR})
add_custom_target(Lucaria
    ALL
    DEPENDS ${CLIENT_BUILD_DIR}/lucaria.html)
add_custom_command(
    TARGET Lucaria
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/client/lucaria.html ${INSTALL_DIR}/client/index.html
    # COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/client/lucaria.data ${INSTALL_DIR}/client/lucaria.data
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/client/lucaria.js ${INSTALL_DIR}/client/lucaria.js
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_BINARY_DIR}/client/lucaria.wasm ${INSTALL_DIR}/client/lucaria.wasm)
