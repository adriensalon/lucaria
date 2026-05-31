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
add_subdirectory(${LUCARIA_EXTERNAL_DIR}/glslang ${CMAKE_CURRENT_BINARY_DIR}/external/glslang)
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
    set(wrapper_cpp "${generated_dir}/${gslc_file_name}")
    file(READ "${gslc_file}" LUCARIA_GSLC_CONTENT)
    configure_file(
        "${LUCARIA_INCLUDE_DIR}/lucaria/core/gsl_template.hpp.in"
        "${embed_hpp}"
        @ONLY)
    set(LUCARIA_GSLC_EMBED_HPP "${embed_hpp}")
    configure_file(
        "${LUCARIA_SOURCE_DIR}/client/core/gsl_template.cpp.in"
        "${wrapper_cpp}"
        @ONLY)
    target_sources(${target} PRIVATE "${wrapper_cpp}")
	set_source_files_properties(${wrapper_cpp} PROPERTIES LANGUAGE CXX)
endfunction()


if(NOT LUCARIA_DISABLE_RELOAD)
	set(LUCARIA_RELOAD_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
	set(LUCARIA_RELOAD_BUILD_DIR "${CMAKE_BINARY_DIR}")
	set(LUCARIA_RELOAD_TARGET "user_game") # TODO
	set(LUCARIA_RELOAD_CMAKE_CONFIGURE_COMMAND
		"cmake -S ${LUCARIA_RELOAD_SOURCE_DIR} -B ${LUCARIA_RELOAD_BUILD_DIR}")
	set(LUCARIA_RELOAD_CMAKE_BUILD_COMMAND
		"cmake --build ${LUCARIA_RELOAD_BUILD_DIR} --target ${LUCARIA_RELOAD_TARGET} --config $<CONFIG>")
	set(LUCARIA_RELOAD_SHARED_LIBRARY_PATH
		"$<TARGET_FILE:${LUCARIA_RELOAD_TARGET}>")
	set(LUCARIA_RELOAD_CACHE_DIRECTORY 
		"${CMAKE_BINARY_DIR}/lucaria_hotreload")
	set(LUCARIA_RELOAD_WATCHED_PATHS "")
	message(${LUCARIA_TARGET_SOURCES})
	foreach(path IN LISTS LUCARIA_TARGET_SOURCES)
		file(TO_CMAKE_PATH "${path}" path_normalized)
		string(APPEND LUCARIA_RELOAD_WATCHED_PATHS "\"${path_normalized}\",\n")
	endforeach()
	configure_file(
		"${LUCARIA_INCLUDE_DIR}/lucaria/core/utils_reload_config.hpp.in"
		"${CMAKE_BINARY_DIR}/generated/lucaria/core/utils_reload_config.hpp"
		@ONLY)
endif()

# lucaria client library
file(GLOB lucaria_client_core_sources "core/*.cpp")
file(GLOB lucaria_client_public_sources "public/*.cpp")
add_library(lucaria_client STATIC ${lucaria_client_core_sources} ${lucaria_client_public_sources})
target_include_directories(lucaria_client PUBLIC ${LUCARIA_INCLUDE_DIR})
set_target_properties(lucaria_client PROPERTIES CXX_STANDARD 17)
target_compile_definitions(lucaria_client PUBLIC BT_NO_SIMD_OPERATOR_OVERLOADS) # ozz implements them already
if(NOT EMSCRIPTEN)
	target_link_libraries(lucaria_client PUBLIC lucaria_external_alsoft)
endif()
	target_link_libraries(lucaria_client PUBLIC lucaria_external_bullet)
	target_link_libraries(lucaria_client PUBLIC lucaria_external_cereal)
	target_link_libraries(lucaria_client PUBLIC lucaria_external_entt)
	target_link_libraries(lucaria_client PUBLIC lucaria_external_glm)
if(NOT PSP)
	target_link_libraries(lucaria_client PUBLIC lucaria_external_glslang)
endif()
if(WIN32)
	target_link_libraries(lucaria_client PUBLIC lucaria_external_glfw)
endif()
target_link_libraries(lucaria_client PUBLIC lucaria_external_imgui)
target_link_libraries(lucaria_client PUBLIC lucaria_external_ozz)
target_link_libraries(lucaria_client PUBLIC lucaria_external_pfr)
target_link_libraries(lucaria_client PUBLIC lucaria_external_stb)
target_link_libraries(lucaria_client PUBLIC lucaria_external_tracy)
target_link_libraries(lucaria_client PUBLIC lucaria_external_vorbis)
target_link_libraries(lucaria_client PUBLIC lucaria_external_woff)

if (MSVC)
	target_compile_options(lucaria_client PRIVATE /bigobj)
endif()

# reload
if(NOT LUCARIA_DISABLE_RELOAD)
	target_include_directories(lucaria_client PUBLIC "${CMAKE_BINARY_DIR}/generated")
endif()


# platform
if(ANDROID)
    message("-- Configuring Lucaria engine for Android platform")
	target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_PLATFORM_ANDROID)
    target_compile_options(lucaria_client PRIVATE -Wno-deprecated-declarations)
	set(NATIVE_APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
    add_library(android_native_app_glue STATIC ${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c)
    target_include_directories(android_native_app_glue PUBLIC ${NATIVE_APP_GLUE_DIR})
    target_link_libraries(lucaria_client PUBLIC EGL android log android_native_app_glue)
	target_sources(lucaria_client PRIVATE
		"core/platform/main_android.cpp"
		"core/platform/manager_window_android.cpp")
elseif(EMSCRIPTEN)
    message("-- Configuring Lucaria engine for Web platform")
	target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_PLATFORM_WEB)
	target_sources(lucaria_client PRIVATE
		"core/platform/main_web.cpp"
		"core/platform/manager_window_web.cpp")
elseif(WIN32) # add macos & linux
    message("-- Configuring Lucaria engine for Win32/MacOS/Linux platform")
	target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_PLATFORM_GLFW)
	target_sources(lucaria_client PRIVATE
		"core/platform/main_glfw.cpp"
		"core/platform/manager_window_glfw.cpp"
		"core/platform/utils_reload_win32.cpp"
		"core/platform/utils_glfw.cpp")
elseif(PSP)
    message("-- Configuring Lucaria engine for PlayStation Portable platform")
	target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_PLATFORM_PSP)
    target_link_libraries(lucaria_client PUBLIC pspdebug pspge pspdisplay)
	target_sources(lucaria_client PRIVATE
		"core/platform/main_psp.cpp"
		"core/platform/manager_window_psp.cpp")
endif()

# generator
message("-- Configuring Lucaria engine with ${CMAKE_GENERATOR} generator")
target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_GENERATOR_NAME=${CMAKE_GENERATOR})

# compiler
message("-- Configuring Lucaria engine with ${CMAKE_CXX_COMPILER_ID} compiler")
target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_COMPILER_NAME=${CMAKE_CXX_COMPILER_ID})

# debug
if(LUCARIA_DEBUG)
    message(STATUS "Configuring Lucaria engine with Debug enabled")
	target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_DEBUG)
else()
    message(STATUS "Configuring Lucaria engine with Debug disabled (${LUCARIA_CONFIG} build)")
endif()

# backend
if(PSP)
    message("-- Configuring Lucaria engine with PSPGU graphics backend")
	target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_BACKEND_PSPGU)
	target_sources(lucaria_client PRIVATE
		"core/backend/object_cubemap_pspgu.cpp"
		"core/backend/object_framebuffer_pspgu.cpp"
		"core/backend/object_mesh_pspgu.cpp"
		"core/backend/object_program_pspgu.cpp"
		"core/backend/object_texture_pspgu.cpp"
		"core/backend/object_renderbuffer_pspgu.cpp")
else()
    message("-- Configuring Lucaria engine with OpenGL graphics backend")
	target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_BACKEND_OPENGL)
	target_sources(lucaria_client PRIVATE
		"core/backend/object_cubemap_opengl.cpp"
		"core/backend/object_framebuffer_opengl.cpp"
		"core/backend/object_mesh_opengl.cpp"
		"core/backend/object_program_opengl.cpp"
		"core/backend/object_texture_opengl.cpp"
		"core/backend/object_renderbuffer_opengl.cpp")
endif()

# simd
if(EMSCRIPTEN)
    message("-- Configuring Lucaria engine with SSE2 over WebAssembly 128-bit SIMD")
    target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_SIMD_SSE20)
elseif(WIN32)
    message("-- Configuring Lucaria engine with SSE4.1 SIMD")
    target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_SIMD_SSE41)
elseif(ANDROID)
    message("-- Configuring Lucaria engine with NEON SIMD")
    target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_SIMD_NEON)
else()
    message("-- Configuring Lucaria engine with SIMD disabled")
    target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_SIMD_OFF)
endif()

# packaged assets
if(LUCARIA_PACKAGE)
    message("-- Configuring Lucaria engine with assets packaging enabled")
    target_compile_definitions(lucaria_client PUBLIC -DLUCARIA_PACKAGED_ASSETS)
else()
    message("-- Configuring Lucaria engine with assets packaging disabled")
endif()
