cmake_minimum_required(VERSION 3.0)
project(lucaria_compiler)

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten")
	message(FATAL_ERROR "CMake is being configured by Emscripten, aborting...")
	return()
	
else()
	set(JUST_INSTALL_CEREAL ON)
	set(SKIP_PERFORMANCE_COMPARISON ON)
	add_subdirectory(../external/cereal ${CMAKE_BINARY_DIR}/external/cereal)
	add_subdirectory(../external/glm ${CMAKE_BINARY_DIR}/external/glm)
	add_subdirectory(../external/stb ${CMAKE_BINARY_DIR}/external/stb)
	add_subdirectory(../external/tinyobjloader ${CMAKE_BINARY_DIR}/external/tinyobjloader)
	# add_subdirectory(../external/tinygltf ${CMAKE_BINARY_DIR}/external/tinygltf)
	set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
	set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
	set(ASSIMP_BUILD_SAMPLES OFF)
	set(ASSIMP_BUILD_TESTS OFF)
	set(BUILD_SHARED_LIBS OFF)
	add_subdirectory(../external/assimp ${CMAKE_BINARY_DIR}/external/assimp)

	add_executable(lucaria_compiler 
		"source/core/mesh.cpp"
		"source/core/shader.cpp"
		"source/core/texture.cpp"
		"source/main.cpp")

	target_link_libraries(lucaria_compiler PRIVATE cereal)
	target_link_libraries(lucaria_compiler PRIVATE glm)
	target_link_libraries(lucaria_compiler PRIVATE stb_image)
	target_link_libraries(lucaria_compiler PRIVATE tinyobjloader)
	# target_link_libraries(lucaria_compiler PRIVATE tinygltf)
	target_link_libraries(lucaria_compiler PRIVATE assimp)

	target_include_directories(lucaria_compiler PRIVATE "${CMAKE_CURRENT_LIST_DIR}/source")
	target_include_directories(lucaria_compiler PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../common/include")

	# target_compile_definitions(lucaria_compiler PRIVATE LUCARIA_JSON)

	set_property(TARGET lucaria_compiler PROPERTY CXX_STANDARD 17)
endif()