cmake_minimum_required(VERSION 3.0)
project(lucaria)

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten")
	set(JUST_INSTALL_CEREAL ON)
	set(SKIP_PERFORMANCE_COMPARISON ON)
	add_subdirectory(../external/cereal ${CMAKE_BINARY_DIR}/external/cereal)
	add_subdirectory(../external/entt ${CMAKE_BINARY_DIR}/external/entt)
	add_subdirectory(../external/glm ${CMAKE_BINARY_DIR}/external/glm)
	add_subdirectory(../external/imgui ${CMAKE_BINARY_DIR}/external/imgui)
	add_subdirectory(../external/ozz ${CMAKE_BINARY_DIR}/external/ozz)

	add_executable(lucaria
		"source/glue/audio.cpp"
		"source/glue/fetch.cpp"
		"source/glue/graphics.cpp"
		"source/glue/update.cpp"
		"source/glue/window.cpp"

		"source/core/cubemap.cpp"
		"source/core/mesh.cpp"
		"source/core/program.cpp"
		"source/core/texture.cpp"
		"source/core/volume.cpp"

		"source/ecs/component/collider.cpp"
		"source/ecs/component/model.cpp"
		"source/ecs/component/portal.cpp"
		"source/ecs/component/speaker.cpp"

		"source/ecs/system/computer.cpp"
		"source/ecs/system/mixer.cpp"
		"source/ecs/system/player.cpp"
		"source/ecs/system/rendering.cpp"
		"source/ecs/system/splash.cpp"
		"source/ecs/system/world.cpp"

		"source/levels/room.cpp"

		"source/main.cpp")

	target_link_libraries(lucaria PRIVATE cereal)
	target_link_libraries(lucaria PRIVATE EnTT)
	target_link_libraries(lucaria PRIVATE glm)
	target_link_libraries(lucaria PRIVATE imgui)
	target_link_libraries(lucaria PRIVATE ozz_animation)

	target_compile_definitions(lucaria PRIVATE LUCARIA_DEBUG)
	# target_compile_definitions(lucaria PRIVATE LUCARIA_JSON)

	target_compile_options(lucaria PRIVATE -Wno-format-security)

	target_include_directories(lucaria PRIVATE "${CMAKE_CURRENT_LIST_DIR}/source")
	target_include_directories(lucaria PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../common/include")

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
	message(FATAL_ERROR "CMake is being configured by Emscripten, aborting...")
	return()
endif()
