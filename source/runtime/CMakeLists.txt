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
	set(LUCARIA_DISABLE_HOTRELOAD ON)
	set(LUCARIA_DISABLE_GUIZMOS ON)
endif()

# lucaria engine/core library, no main() here
file(GLOB lucaria_runtime_core_sources "core/*.cpp")
file(GLOB lucaria_runtime_engine_sources "engine/*.cpp")
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
        "core/platform/manager_window_android.cpp")
    set(lucaria_runtime_main_sources "core/platform/main_android.cpp")
elseif(EMSCRIPTEN)
    message("-- Configuring Lucaria engine for Web platform")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PLATFORM_WEB 
        -DLUCARIA_DISABLE_COMPUTE_SPIRV)
    list(APPEND lucaria_runtime_platform_sources
        "core/platform/manager_window_web.cpp")
    set(lucaria_runtime_main_sources "core/platform/main_web.cpp")
elseif(WIN32)
    message("-- Configuring Lucaria engine for Win32/MacOS/Linux platform")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PLATFORM_GLFW)
    list(APPEND lucaria_runtime_platform_sources
        "core/platform/manager_window_glfw.cpp"
        "core/platform/utils_reload_win32.cpp"
        "core/platform/utils_glfw.cpp")
    set(lucaria_runtime_main_sources "core/platform/main_glfw.cpp")
elseif(PSP)
    message("-- Configuring Lucaria engine for PlayStation Portable platform")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_PLATFORM_PSP)
    target_link_libraries(lucaria_runtime PUBLIC pspdebug pspge pspdisplay)
    list(APPEND lucaria_runtime_platform_sources
        "core/platform/manager_window_psp.cpp")
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
        "core/backend/object_cubemap_pspgu.cpp"
        "core/backend/object_framebuffer_pspgu.cpp"
        "core/backend/object_mesh_pspgu.cpp"
        "core/backend/object_program_pspgu.cpp"
        "core/backend/object_texture_pspgu.cpp"
        "core/backend/object_renderbuffer_pspgu.cpp")
else()
    message("-- Configuring Lucaria engine with OpenGL graphics backend")
    target_compile_definitions(lucaria_runtime PUBLIC -DLUCARIA_BACKEND_OPENGL)
    target_sources(lucaria_runtime PRIVATE
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
