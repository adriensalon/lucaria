cmake_minimum_required(VERSION 3.13)
project(lucaria_game LANGUAGES C CXX)

if(NOT LUCARIA_GAME_DIR)
    message(FATAL_ERROR "LUCARIA_GAME_DIR is required")
endif()
if(NOT EXISTS "${LUCARIA_GAME_DIR}/build.cmake")
    message(FATAL_ERROR "Missing game build script: ${LUCARIA_GAME_DIR}/build.cmake")
endif()

include("${LUCARIA_CMAKE_DIR}/build_api.cmake")
add_subdirectory("${LUCARIA_SOURCE_DIR}/runtime" "${CMAKE_CURRENT_BINARY_DIR}/runtime")

set(LUCARIA_USER_LIBRARY_TYPE STATIC)
if(NOT LUCARIA_DISABLE_RELOAD AND NOT ANDROID AND NOT EMSCRIPTEN AND NOT PSP)
	set(LUCARIA_USER_LIBRARY_TYPE SHARED)
endif()
add_library(lucaria_game ${LUCARIA_USER_LIBRARY_TYPE})

set(GAME_DIR ${LUCARIA_GAME_DIR})
include("${LUCARIA_GAME_DIR}/build.cmake")
if(NOT LUCARIA_TARGET)
    message(FATAL_ERROR "build.cmake must call add_game(<output_name> SOURCES ...)")
endif()

get_target_property(LUCARIA_TARGET_SOURCES lucaria_game SOURCES)
if(NOT LUCARIA_TARGET_SOURCES)
	message(FATAL_ERROR "lucaria_make_finalize requires LUCARIA_TARGET_SOURCES")
endif()
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
if(WIN32 AND LUCARIA_HIDE_CONSOLE)
	target_compile_definitions(lucaria_main PRIVATE LUCARIA_HIDE_CONSOLE=1)
	set_target_properties(lucaria_main PROPERTIES WIN32_EXECUTABLE TRUE)
else()
	target_compile_definitions(lucaria_main PRIVATE LUCARIA_HIDE_CONSOLE=0)
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

