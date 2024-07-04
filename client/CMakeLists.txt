cmake_minimum_required(VERSION 3.0)
project(lucaria)

function(message)
	if(NOT MESSAGE_QUIET)
		_message(${ARGN})
	endif()
endfunction()

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten")
	set(MESSAGE_QUIET ON)
		add_subdirectory(../external/entt ${CMAKE_BINARY_DIR}/external/entt)
		add_subdirectory(../external/glm ${CMAKE_BINARY_DIR}/external/glm)
		add_subdirectory(../external/imgui ${CMAKE_BINARY_DIR}/external/imgui)

		set(JUST_INSTALL_CEREAL ON)
		set(SKIP_PERFORMANCE_COMPARISON ON)
		add_subdirectory(../external/cereal ${CMAKE_BINARY_DIR}/external/cereal)

		set(ozz_build_tools OFF)
		set(ozz_build_fbx OFF)
		set(ozz_build_gltf OFF)
		set(ozz_build_data OFF)
		set(ozz_build_postfix OFF)
		set(ozz_run_tests_headless OFF)
		add_subdirectory(../external/ozz ${CMAKE_BINARY_DIR}/external/ozz)
		target_compile_options(ozz_base PRIVATE "-Wno-#pragma-messages")
	set(MESSAGE_QUIET OFF)

	file(GLOB_RECURSE lucaria_sources "${CMAKE_CURRENT_LIST_DIR}/source/*.cpp")
	add_executable(lucaria ${lucaria_sources})

	target_include_directories(lucaria PRIVATE ${CMAKE_CURRENT_LIST_DIR}/source)
	target_include_directories(lucaria PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../common/include)

	target_link_libraries(lucaria PRIVATE cereal)
	target_link_libraries(lucaria PRIVATE EnTT)
	target_link_libraries(lucaria PRIVATE glm)
	target_link_libraries(lucaria PRIVATE imgui)
	target_link_libraries(lucaria PRIVATE ozz_animation)

	target_compile_definitions(lucaria PRIVATE LUCARIA_DEBUG)

	# target_compile_definitions(lucaria PRIVATE LUCARIA_JSON)
	target_compile_options(lucaria PRIVATE -Wno-format-security)

	set_target_properties(lucaria PROPERTIES CXX_STANDARD 17)

	set(CXX_STANDARD ".html")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--use-preload-cache")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
		--shell-file ${CMAKE_CURRENT_LIST_DIR}/source/shell.html \
		-s LLD_REPORT_UNDEFINED \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s MAX_WEBGL_VERSION=2 \
		-s USE_WEBGL2=1 \
		-s GL_DEBUG=1 \
		-s FETCH \
		-s NO_DISABLE_EXCEPTION_CATCHING \
		-lopenal")

# -O3 \
# --preload-file ${CMAKE_CURRENT_LIST_DIR}/../install/assets@/ \
else()
	message(FATAL_ERROR "CMake is being configured by host platform but we require Emscripten, aborting...")
	return()
endif()
