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
	
	set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
	set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
	# set(ASSIMP_BUILD_BLEND_IMPORTER ON)     # Blender importer
	# set(ASSIMP_BUILD_OBJ_IMPORTER ON)       # OBJ importer
	# set(ASSIMP_BUILD_FBX_IMPORTER ON)       # FBX importer
	set(ASSIMP_BUILD_GLTF_IMPORTER ON)      # GLTF importer
	# set(ASSIMP_BUILD_GLTF_EXPORTER ON)      # GLTF exporter
	set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
	set(ASSIMP_BUILD_TESTS OFF)
	set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
	set(ASSIMP_BUILD_SAMPLES OFF)
	set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
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
	target_link_libraries(lucaria_compiler PRIVATE assimp)

	target_include_directories(lucaria_compiler PRIVATE "${CMAKE_CURRENT_LIST_DIR}/source")
	target_include_directories(lucaria_compiler PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../common/include")

	# target_compile_definitions(lucaria_compiler PRIVATE LUCARIA_JSON)

	set_property(TARGET lucaria_compiler PROPERTY CXX_STANDARD 17)
endif()