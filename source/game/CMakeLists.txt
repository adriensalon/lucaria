cmake_minimum_required(VERSION 3.19)
project(lucaria_game LANGUAGES C CXX)

if(NOT LUCARIA_GAME_DIR)
    message(FATAL_ERROR "LUCARIA_GAME_DIR is required")
endif()
set(LUCARIA_GAME_CONFIG "${LUCARIA_GAME_DIR}/build.json")
if(NOT EXISTS "${LUCARIA_GAME_CONFIG}")
    message(FATAL_ERROR "Missing game build config: ${LUCARIA_GAME_CONFIG}")
endif()
file(READ "${LUCARIA_GAME_CONFIG}" LUCARIA_GAME_JSON)

function(_lucaria_read_json_string_array OUT_VAR FIELD REQUIRED)
    set(values)
    string(JSON value_type ERROR_VARIABLE value_type_error
        TYPE "${LUCARIA_GAME_JSON}" game ${FIELD})
    if(value_type_error)
        if(REQUIRED)
            message(FATAL_ERROR
                "Missing required build.json field: game.${FIELD}: ${value_type_error}")
        endif()
        set(${OUT_VAR} "" PARENT_SCOPE)
        return()
    endif()
    if(NOT value_type STREQUAL "ARRAY")
        message(FATAL_ERROR "build.json field game.${FIELD} must be an array")
    endif()

    string(JSON value_count ERROR_VARIABLE value_count_error
        LENGTH "${LUCARIA_GAME_JSON}" game ${FIELD})
    if(value_count_error)
        message(FATAL_ERROR
            "Could not read build.json field game.${FIELD}: ${value_count_error}")
    endif()
    if(value_count GREATER 0)
        math(EXPR value_last "${value_count} - 1")
        foreach(value_index RANGE 0 ${value_last})
            string(JSON value ERROR_VARIABLE value_error
                GET "${LUCARIA_GAME_JSON}" game ${FIELD} ${value_index})
            if(value_error)
                message(FATAL_ERROR
                    "Could not read build.json field game.${FIELD}[${value_index}]: ${value_error}")
            endif()
            list(APPEND values "${value}")
        endforeach()
    endif()
    set(${OUT_VAR} ${values} PARENT_SCOPE)
endfunction()

function(_lucaria_make_game_path OUT_VAR VALUE)
    if(IS_ABSOLUTE "${VALUE}")
        set(${OUT_VAR} "${VALUE}" PARENT_SCOPE)
    else()
        set(${OUT_VAR} "${LUCARIA_GAME_DIR}/${VALUE}" PARENT_SCOPE)
    endif()
endfunction()

# external libraries
if(NOT EMSCRIPTEN)
    add_subdirectory(${LUCARIA_EXTERNAL_DIR}/alsoft ${CMAKE_CURRENT_BINARY_DIR}/external/alsoft)
endif()
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/bullet ${CMAKE_CURRENT_BINARY_DIR}/external/bullet)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/cereal ${CMAKE_CURRENT_BINARY_DIR}/external/cereal)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/entt ${CMAKE_CURRENT_BINARY_DIR}/external/entt)
if(WIN32)
    add_subdirectory(${LUCARIA_EXTERNAL_DIR}/glfw ${CMAKE_CURRENT_BINARY_DIR}/external/glfw)
endif()
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/glm ${CMAKE_CURRENT_BINARY_DIR}/external/glm)
if(NOT PSP AND NOT EMSCRIPTEN)
    add_subdirectory(${LUCARIA_EXTERNAL_DIR}/glslang ${CMAKE_CURRENT_BINARY_DIR}/external/glslang)
endif()
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/imgui ${CMAKE_CURRENT_BINARY_DIR}/external/imgui)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/ogg ${CMAKE_CURRENT_BINARY_DIR}/external/ogg)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/ozz ${CMAKE_CURRENT_BINARY_DIR}/external/ozz)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/pfr ${CMAKE_CURRENT_BINARY_DIR}/external/pfr)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/stb ${CMAKE_CURRENT_BINARY_DIR}/external/stb)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/tracy ${CMAKE_CURRENT_BINARY_DIR}/external/tracy)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/vorbis ${CMAKE_CURRENT_BINARY_DIR}/external/vorbis)
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/woff ${CMAKE_CURRENT_BINARY_DIR}/external/woff)

function(embed_gslc target gslc_file)
    get_filename_component(gslc_file_name "${gslc_file}" NAME)
    string(SHA1 LUCARIA_GSLC_ID_HASH "${gslc_file}")
    string(SUBSTRING "${LUCARIA_GSLC_ID_HASH}" 0 16 LUCARIA_GSLC_ID)
    string(MAKE_C_IDENTIFIER "gsl_template_${LUCARIA_GSLC_ID}" LUCARIA_GSLC_IDENT)
    set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/lucaria/core")
    file(MAKE_DIRECTORY "${generated_dir}")
    set(LUCARIA_GSLC_FILE "${gslc_file}")
    set(embed_hpp "${generated_dir}/${LUCARIA_GSLC_IDENT}.hpp")
    set(wrapper_cpp "${generated_dir}/${gslc_file_name}.cpp")
    file(READ "${gslc_file}" LUCARIA_GSLC_CONTENT)
    configure_file(
        "${LUCARIA_INCLUDE_DIR}/lucaria/core/gsl_template.hpp.in"
        "${embed_hpp}"
        @ONLY)
    set(LUCARIA_GSLC_EMBED_HPP "${embed_hpp}")
    configure_file(
        "${LUCARIA_SOURCE_DIR}/runtime/core/gsl_template.cpp.in"
        "${wrapper_cpp}"
        @ONLY)
    target_sources(${target} PUBLIC "${wrapper_cpp}")
endfunction()

# non desktop platforms dont enable code hotreload for now
if(PSP OR ANDROID OR EMSCRIPTEN)
	set(LUCARIA_DISABLE_RELOAD ON)
endif()

# release builds strip maximum code
if(NOT LUCARIA_GAME_DEBUG)
	set(LUCARIA_DISABLE_DEBUG ON)
	set(LUCARIA_DISABLE_RELOAD ON)
	set(LUCARIA_DISABLE_GUIZMOS ON)
endif()

# lucaria engine/core library, no main() here
file(GLOB lucaria_runtime_core_sources "${LUCARIA_SOURCE_DIR}/runtime/core/*.cpp")
file(GLOB lucaria_runtime_engine_sources "${LUCARIA_SOURCE_DIR}/runtime/engine/*.cpp")
set(lucaria_runtime_sources
    ${lucaria_runtime_core_sources}
    ${lucaria_runtime_engine_sources})
	
if(NOT LUCARIA_DISABLE_RELOAD)
	add_library(lucaria_runtime SHARED ${lucaria_runtime_sources})
	if(WIN32)
		set_target_properties(lucaria_runtime PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
	endif()
else()
	add_library(lucaria_runtime STATIC ${lucaria_runtime_sources})
endif()
target_include_directories(lucaria_runtime PUBLIC ${LUCARIA_INCLUDE_DIR})
set_target_properties(lucaria_runtime PROPERTIES CXX_STANDARD 17)
target_compile_definitions(lucaria_runtime PUBLIC BT_NO_SIMD_OPERATOR_OVERLOADS) # ozz implements them already
if(NOT EMSCRIPTEN)
    target_link_libraries(lucaria_runtime PUBLIC lucaria_external_alsoft)
endif()
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_bullet)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_cereal)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_entt)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_glm)
if(NOT PSP AND NOT EMSCRIPTEN)
    target_link_libraries(lucaria_runtime PUBLIC lucaria_external_glslang)
endif()
if(WIN32)
    target_link_libraries(lucaria_runtime PUBLIC lucaria_external_glfw)
endif()
target_link_libraries(lucaria_runtime PRIVATE lucaria_external_imgui)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_imgui_interface)
if(WIN32 AND NOT LUCARIA_DISABLE_RELOAD)
    target_compile_definitions(lucaria_runtime PRIVATE "IMGUI_API=__declspec(dllexport)")
    target_link_options(lucaria_runtime PRIVATE "/WHOLEARCHIVE:$<TARGET_FILE:lucaria_external_imgui>")
endif()
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_ozz)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_pfr)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_stb)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_tracy)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_vorbis)
target_link_libraries(lucaria_runtime PUBLIC lucaria_external_woff)

if (MSVC)
    target_compile_options(lucaria_runtime PRIVATE /bigobj)
endif()

# if(NOT LUCARIA_DISABLE_RELOAD)
#     target_include_directories(lucaria_runtime PUBLIC "${CMAKE_BINARY_DIR}/generated/$<CONFIG>")
# endif()

set(lucaria_runtime_platform_sources "")
set(lucaria_runtime_main_sources "")

# platform
if(ANDROID)
    message("-- Configuring Lucaria engine for Android platform")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PLATFORM_ANDROID)
    target_compile_options(lucaria_runtime PRIVATE -Wno-deprecated-declarations)
    set(NATIVE_APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
    add_library(android_native_app_glue STATIC ${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c)
    target_include_directories(android_native_app_glue PUBLIC ${NATIVE_APP_GLUE_DIR})
    target_link_libraries(lucaria_runtime PUBLIC EGL android log android_native_app_glue)
    list(APPEND lucaria_runtime_platform_sources
        "${LUCARIA_SOURCE_DIR}/runtime/core/platform/manager_window_android.cpp")
    set(lucaria_runtime_main_sources "core/platform/main_android.cpp")
elseif(EMSCRIPTEN)
    message("-- Configuring Lucaria engine for Web platform")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PLATFORM_WEB 
        -DLUCARIA_DISABLE_COMPUTE_SPIRV)
    list(APPEND lucaria_runtime_platform_sources
        "${LUCARIA_SOURCE_DIR}/runtime/core/platform/manager_window_web.cpp")
    set(lucaria_runtime_main_sources "core/platform/main_web.cpp")
elseif(WIN32)
    message("-- Configuring Lucaria engine for Win32/MacOS/Linux platform")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PLATFORM_WIN32)
    list(APPEND lucaria_runtime_platform_sources
        "${LUCARIA_SOURCE_DIR}/runtime/core/platform/manager_window_glfw.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/platform/utils_reload_win32.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/platform/utils_glfw.cpp")
    set(lucaria_runtime_main_sources "${LUCARIA_SOURCE_DIR}/runtime/core/platform/main_glfw.cpp")
elseif(PSP)
    message("-- Configuring Lucaria engine for PlayStation Portable platform")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PLATFORM_PSP)
    target_link_libraries(lucaria_runtime PUBLIC pspdebug pspge pspdisplay)
    list(APPEND lucaria_runtime_platform_sources
        "${LUCARIA_SOURCE_DIR}/runtime/core/platform/manager_window_psp.cpp")
    set(lucaria_runtime_main_sources "core/platform/main_psp.cpp")
endif()
target_sources(lucaria_runtime PRIVATE ${lucaria_runtime_platform_sources})

# lucaria runtime executable owns main()
if(ANDROID)
    add_library(lucaria_main SHARED ${lucaria_runtime_main_sources})
else()
    add_executable(lucaria_main ${lucaria_runtime_main_sources})
endif()
set_target_properties(lucaria_main PROPERTIES CXX_STANDARD 17)
target_link_libraries(lucaria_main PRIVATE lucaria_runtime)

# generator
message("-- Configuring Lucaria engine with ${CMAKE_GENERATOR} generator")
target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_GENERATOR_NAME=${CMAKE_GENERATOR})

# compiler
message("-- Configuring Lucaria engine with ${CMAKE_CXX_COMPILER_ID} compiler")
target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_COMPILER_NAME=${CMAKE_CXX_COMPILER_ID})

# debug
if(LUCARIA_GAME_DEBUG)
    message(STATUS "Configuring Lucaria engine with Debug enabled")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_DEBUG)
else()
    message(STATUS "Configuring Lucaria engine with Debug disabled (${LUCARIA_CONFIG} build)")
endif()

# backend
if(PSP)
    message("-- Configuring Lucaria engine with PSPGU graphics backend")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_BACKEND_PSPGU)
    target_sources(lucaria_runtime PRIVATE
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_cubemap_pspgu.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_framebuffer_pspgu.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_mesh_pspgu.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_program_pspgu.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_texture_pspgu.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_renderbuffer_pspgu.cpp")
else()
    message("-- Configuring Lucaria engine with OpenGL graphics backend")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_BACKEND_OPENGL)
    target_sources(lucaria_runtime PRIVATE
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_storage_opengl.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_cubemap_opengl.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_framebuffer_opengl.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_mesh_opengl.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_program_opengl.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_texture_opengl.cpp"
        "${LUCARIA_SOURCE_DIR}/runtime/core/backend/rendering_renderbuffer_opengl.cpp")
endif()

# simd
if(EMSCRIPTEN)
    message("-- Configuring Lucaria engine with SSE2 over WebAssembly 128-bit SIMD")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_SIMD_SSE20)
elseif(WIN32)
    message("-- Configuring Lucaria engine with SSE4.1 SIMD")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_SIMD_SSE41)
elseif(ANDROID)
    message("-- Configuring Lucaria engine with NEON SIMD")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_SIMD_NEON)
else()
    message("-- Configuring Lucaria engine with SIMD disabled")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_SIMD_OFF)
endif()

# packaged assets
if(LUCARIA_PACKAGE)
    message("-- Configuring Lucaria engine with assets packaging enabled")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PACKAGED_ASSETS)
else()
    message("-- Configuring Lucaria engine with assets packaging disabled")
endif()

set(LUCARIA_USER_LIBRARY_TYPE STATIC)
if(NOT LUCARIA_DISABLE_RELOAD AND NOT ANDROID AND NOT EMSCRIPTEN AND NOT PSP)
	set(LUCARIA_USER_LIBRARY_TYPE SHARED)
endif()
add_library(lucaria_game ${LUCARIA_USER_LIBRARY_TYPE})

string(JSON LUCARIA_TARGET ERROR_VARIABLE lucaria_target_error
    GET "${LUCARIA_GAME_JSON}" game name)
if(lucaria_target_error)
    message(FATAL_ERROR
        "Missing required build.json field: game.name: ${lucaria_target_error}")
endif()
if(NOT LUCARIA_TARGET)
    message(FATAL_ERROR "build.json field game.name must not be empty")
endif()
add_library(${LUCARIA_TARGET} ALIAS lucaria_game)

_lucaria_read_json_string_array(lucaria_source_patterns sources TRUE)
set(LUCARIA_TARGET_SOURCES)
foreach(lucaria_source_pattern IN LISTS lucaria_source_patterns)
    _lucaria_make_game_path(lucaria_source_glob "${lucaria_source_pattern}")
    file(GLOB_RECURSE lucaria_source_matches
        CONFIGURE_DEPENDS
        "${lucaria_source_glob}")
    if(NOT lucaria_source_matches)
        message(FATAL_ERROR
            "build.json source pattern matched no files: ${lucaria_source_pattern}")
    endif()
    list(APPEND LUCARIA_TARGET_SOURCES ${lucaria_source_matches})
endforeach()
if(NOT LUCARIA_TARGET_SOURCES)
	message(FATAL_ERROR "build.json field game.sources must contain at least one source")
endif()
_lucaria_read_json_string_array(lucaria_include_dirs include_dirs FALSE)
set(LUCARIA_TARGET_INCLUDE_DIRS)
foreach(lucaria_include_dir IN LISTS lucaria_include_dirs)
    _lucaria_make_game_path(lucaria_include_path "${lucaria_include_dir}")
    list(APPEND LUCARIA_TARGET_INCLUDE_DIRS "${lucaria_include_path}")
endforeach()
_lucaria_read_json_string_array(LUCARIA_TARGET_DEFINES compile_definitions FALSE)
_lucaria_read_json_string_array(LUCARIA_TARGET_BUILD_ARGS compile_options FALSE)

if(NOT TARGET lucaria_runtime)
	message(FATAL_ERROR "lucaria_make_finalize requires target lucaria_runtime")
endif()
if(NOT TARGET lucaria_main)
	message(FATAL_ERROR "lucaria_make_finalize requires target lucaria_main")
endif()

# Hot reload config
if(NOT LUCARIA_DISABLE_RELOAD)
	set(LUCARIA_RELOAD_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
	set(LUCARIA_RELOAD_BUILD_DIR "${CMAKE_BINARY_DIR}")
	set(LUCARIA_RELOAD_TARGET "lucaria_game")
	set(LUCARIA_RELOAD_CMAKE_CONFIGURE_COMMAND "cmake -S ${LUCARIA_RELOAD_SOURCE_DIR} -B ${LUCARIA_RELOAD_BUILD_DIR}")
	set(LUCARIA_RELOAD_CMAKE_BUILD_COMMAND "cmake --build ${LUCARIA_RELOAD_BUILD_DIR} --target lucaria_game --config $<CONFIG>")
	set(LUCARIA_RELOAD_SHARED_LIBRARY_PATH "$<TARGET_FILE:lucaria_game>")
	set(LUCARIA_RELOAD_CACHE_DIRECTORY "${CMAKE_BINARY_DIR}/lucaria_hotreload")
	set(LUCARIA_RELOAD_WATCHED_PATHS "")
	foreach(path IN LISTS LUCARIA_TARGET_SOURCES)
		file(TO_CMAKE_PATH "${path}" path_normalized)
		string(APPEND LUCARIA_RELOAD_WATCHED_PATHS "        \"${path_normalized}\",\n")
	endforeach()
	set(LUCARIA_RELOAD_CONFIG_TEMPLATE
		"${CMAKE_BINARY_DIR}/generated/lucaria/core/reload_config.hpp.gen")
	configure_file(
		"${LUCARIA_INCLUDE_DIR}/lucaria/core/reload_config.hpp.in"
		"${LUCARIA_RELOAD_CONFIG_TEMPLATE}"
		@ONLY)
	file(GENERATE
		OUTPUT "${CMAKE_BINARY_DIR}/generated/$<CONFIG>/lucaria/core/reload_config.hpp"
		INPUT "${LUCARIA_RELOAD_CONFIG_TEMPLATE}")
	target_include_directories(lucaria_runtime PUBLIC
		"${CMAKE_BINARY_DIR}/generated/$<CONFIG>")
else()
	target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_DISABLE_RELOAD)
endif()

# split normal C++ sources from embedded GSL sources
set(LUCARIA_GSLC_SOURCES ${LUCARIA_TARGET_SOURCES})
list(FILTER LUCARIA_GSLC_SOURCES INCLUDE REGEX "\\.gslc$")
set(LUCARIA_NORMAL_SOURCES ${LUCARIA_TARGET_SOURCES})
list(FILTER LUCARIA_NORMAL_SOURCES EXCLUDE REGEX "\\.gslc$")
if(NOT LUCARIA_NORMAL_SOURCES)
	message(FATAL_ERROR "add_lucaria_game(${LUCARIA_TARGET}) must contain at least one non-.gslc source")
endif()

# user game module
target_sources(lucaria_game PRIVATE ${LUCARIA_NORMAL_SOURCES})    
foreach(gslc_source IN LISTS LUCARIA_GSLC_SOURCES)
	embed_gslc(lucaria_game "${gslc_source}")
endforeach()
set_target_properties(lucaria_game PROPERTIES CXX_STANDARD 17)
target_link_libraries(lucaria_game PRIVATE lucaria_runtime)
if(LUCARIA_TARGET_INCLUDE_DIRS)
	target_include_directories(lucaria_game PRIVATE ${LUCARIA_TARGET_INCLUDE_DIRS})
endif()
if(LUCARIA_TARGET_DEFINES)
	target_compile_definitions(lucaria_game PRIVATE ${LUCARIA_TARGET_DEFINES})
endif()
if(LUCARIA_TARGET_BUILD_ARGS)
	target_compile_options(lucaria_game PRIVATE ${LUCARIA_TARGET_BUILD_ARGS})
endif()

# main executable/shared lib output name
set_target_properties(lucaria_main PROPERTIES OUTPUT_NAME "${LUCARIA_TARGET}")
if(NOT LUCARIA_DISABLE_RELOAD AND NOT ANDROID AND NOT EMSCRIPTEN AND NOT PSP)
	add_dependencies(lucaria_main lucaria_game)
else()
	target_link_libraries(lucaria_main PRIVATE lucaria_game)
endif()

# PSP package
if(PSP)
	create_pbp_file(
		TARGET lucaria_main
		ICON_PATH NULL
		BACKGROUND_PATH NULL
		PREVIEW_PATH NULL
		TITLE ${LUCARIA_TARGET}
		VERSION 01.00)
endif()

# Web final link options
if(EMSCRIPTEN)
	if(NOT LUCARIA_HTML_SHELL)
		set(LUCARIA_HTML_SHELL "${LUCARIA_SOURCE_DIR}/game/web/shell.html")
	endif()
	set_target_properties(lucaria_main PROPERTIES SUFFIX ".html")
	target_link_options(lucaria_main PRIVATE
		--shell-file "${LUCARIA_HTML_SHELL}"
		-sLLD_REPORT_UNDEFINED
		-sALLOW_MEMORY_GROWTH=1
		-sINITIAL_MEMORY=671088640
		-sMALLOC=emmalloc
		-sMAX_WEBGL_VERSION=2
		-sUSE_WEBGL2=1
		-sFETCH
		-lopenal)
	if(LUCARIA_GAME_DEBUG)
		target_link_options(lucaria_main PRIVATE
			-g
			-sGL_DEBUG=1
			-sNO_DISABLE_EXCEPTION_CATCHING)
	else()
		target_link_options(lucaria_main PRIVATE
			-O3
			-sGL_DEBUG=0)
	endif()
endif()

# Win32 console
if(WIN32 AND NOT LUCARIA_GAME_DEBUG)
	target_compile_definitions(lucaria_main PRIVATE -DLUCARIA_HIDE_CONSOLE)
	set_target_properties(lucaria_main PROPERTIES WIN32_EXECUTABLE TRUE)
else()
endif()

# IPO / LTO
cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)
check_ipo_supported(RESULT _lucaria_ipo_supported OUTPUT _lucaria_ipo_error)
function(lucaria_enable_ipo_recursive TARGET_NAME)
	if(NOT TARGET ${TARGET_NAME})
		return()
	endif()
	get_property(already_set TARGET ${TARGET_NAME} PROPERTY _LUCARIA_IPO_MARKER SET)
	if(already_set)
		return()
	endif()
	set_property(TARGET ${TARGET_NAME} PROPERTY _LUCARIA_IPO_MARKER TRUE)
	set_property(TARGET ${TARGET_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	get_target_property(link_libs ${TARGET_NAME} LINK_LIBRARIES)
	if(NOT link_libs)
		return()
	endif()
	foreach(lib IN LISTS link_libs)
		if(TARGET ${lib})
			lucaria_enable_ipo_recursive(${lib})
		endif()
	endforeach()
endfunction()

if(NOT LUCARIA_GAME_DEBUG AND LUCARIA_DISABLE_RELOAD)
	if(_lucaria_ipo_supported)
		lucaria_enable_ipo_recursive(lucaria_main)
		lucaria_enable_ipo_recursive(lucaria_game)
	else()
		message(STATUS "IPO/LTO not supported: ${_lucaria_ipo_error}")
	endif()
endif()

