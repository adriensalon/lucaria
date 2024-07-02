cmake_minimum_required(VERSION 3.0)
project(lucaria)

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten")
	set(JUST_INSTALL_CEREAL ON)
	set(SKIP_PERFORMANCE_COMPARISON ON)
	add_subdirectory(../external/cereal ${CMAKE_BINARY_DIR}/external/cereal)
	add_subdirectory(../external/glm ${CMAKE_BINARY_DIR}/external/glm)
	add_subdirectory(../external/imgui ${CMAKE_BINARY_DIR}/external/imgui)

	add_executable(lucaria
		"source/glue/audio.cpp"
		"source/glue/fetch.cpp"
		"source/glue/graphics.cpp"
		"source/glue/window.cpp"

		"source/core/cubemap.cpp"
		"source/core/mesh.cpp"
		"source/core/program.cpp"
		"source/core/texture.cpp"
		"source/core/volume.cpp"

		"source/scene/camera.cpp"
		"source/scene/computer.cpp"
		"source/scene/controller.cpp"
		"source/scene/room.cpp"
		"source/scene/screen.cpp"
		"source/scene/skybox.cpp"
		"source/scene/speaker.cpp"
		"source/scene/splash.cpp"

		"source/main.cpp")

	target_link_libraries(lucaria PRIVATE cereal)
	target_link_libraries(lucaria PRIVATE glm)
	target_link_libraries(lucaria PRIVATE imgui)

	target_compile_definitions(lucaria PRIVATE LUCARIA_DEBUG)

	target_include_directories(lucaria PRIVATE "${CMAKE_CURRENT_LIST_DIR}/source")
	target_include_directories(lucaria PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../common/include")

	set_target_properties(lucaria PROPERTIES CXX_STANDARD 17)

	set(CXX_STANDARD ".html")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--use-preload-cache")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
		-O2
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
	message(FATAL_ERROR "CMake is being configured by Emscripten, aborting...")
	return()
endif()
