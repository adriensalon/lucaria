cmake_minimum_required(VERSION 3.12)
project(lucaria_compiler)

cmake_policy(SET CMP0077 NEW)

function(message)
	if(NOT MESSAGE_QUIET)
		_message(${ARGN})
	endif()
endfunction()

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten")
	message(FATAL_ERROR "CMake is being configured by Emscripten, aborting...")
	return()
else()
	set(MESSAGE_QUIET ON)
		add_subdirectory(../external/glm ${CMAKE_BINARY_DIR}/external/glm)
		add_subdirectory(../external/stb ${CMAKE_BINARY_DIR}/external/stb)
				
		add_subdirectory(../external/ogg ${CMAKE_BINARY_DIR}/external/ogg)
		set(OGG_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/ogg/include)
		set(OGG_LIBRARIES ogg)

		option(INSTALL_CMAKE_PACKAGE_MODULE OFF)
		add_subdirectory(../external/vorbis ${CMAKE_BINARY_DIR}/external/vorbis)
		set(VORBIS_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/vorbis/include)
		if (MSVC)
			target_compile_options(vorbis PRIVATE /wd4244)
			target_compile_options(vorbis PRIVATE /wd4267)
			target_compile_options(vorbis PRIVATE /wd4305)
			target_compile_options(vorbisenc PRIVATE /wd4244)
			target_compile_options(vorbisenc PRIVATE /wd4267)
			target_compile_options(vorbisenc PRIVATE /wd4305)
			target_compile_options(vorbisfile PRIVATE /wd4244)
			target_compile_options(vorbisfile PRIVATE /wd4267)
			target_compile_options(vorbisfile PRIVATE /wd4305)
		endif()
		
		add_subdirectory(../external/vorbistools ${CMAKE_BINARY_DIR}/external/vorbistools)
		target_include_directories(oggenc PRIVATE ${OGG_INCLUDE_DIRS})
		target_include_directories(oggenc PRIVATE ${VORBIS_INCLUDE_DIRS})

		set(JUST_INSTALL_CEREAL ON)
		set(SKIP_PERFORMANCE_COMPARISON ON)
		add_subdirectory(../external/cereal ${CMAKE_BINARY_DIR}/external/cereal)

		set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
		set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
		set(ASSIMP_BUILD_GLTF_IMPORTER ON)
		set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
		set(ASSIMP_BUILD_TESTS OFF)
		set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
		set(ASSIMP_BUILD_SAMPLES OFF)
		set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
		set(BUILD_SHARED_LIBS OFF)
		add_subdirectory(../external/assimp ${CMAKE_BINARY_DIR}/external/assimp)

		add_subdirectory(../external/etcpak ${CMAKE_BINARY_DIR}/external/etcpak) # we use zlib from assimp

		set(ozz_build_tools ON)
		set(ozz_build_fbx OFF)
		set(ozz_build_gltf ON)
		set(ozz_build_data OFF)
		set(ozz_build_postfix OFF)
		set(ozz_run_tests_headless OFF)
		add_subdirectory(../external/ozz ${CMAKE_BINARY_DIR}/external/ozz)
	set(MESSAGE_QUIET OFF)

	file(GLOB_RECURSE lucaria_compiler_sources "${CMAKE_CURRENT_LIST_DIR}/source/*.cpp")
	add_executable(lucaria_compiler ${lucaria_compiler_sources})

	target_include_directories(lucaria_compiler PRIVATE ${CMAKE_CURRENT_LIST_DIR}/source)
	target_include_directories(lucaria_compiler PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../common/include)
	target_include_directories(lucaria_compiler PRIVATE ${OGG_INCLUDE_DIRS})
	target_include_directories(lucaria_compiler PRIVATE ${VORBIS_INCLUDE_DIRS})

	target_link_libraries(lucaria_compiler PRIVATE cereal)
	target_link_libraries(lucaria_compiler PRIVATE glm)
	target_link_libraries(lucaria_compiler PRIVATE ogg)
	target_link_libraries(lucaria_compiler PRIVATE stb_image)
	target_link_libraries(lucaria_compiler PRIVATE assimp)
	target_link_libraries(lucaria_compiler PRIVATE vorbis vorbisenc vorbisfile)

	# target_compile_definitions(lucaria_compiler PRIVATE LUCARIA_DEBUG)
	# target_compile_definitions(lucaria_compiler PRIVATE LUCARIA_JSON)
	set_target_properties(lucaria_compiler PROPERTIES CXX_STANDARD 17)

	add_dependencies(lucaria_compiler etcpak)
	add_dependencies(lucaria_compiler gltf2ozz)
	add_dependencies(lucaria_compiler oggenc)

	add_custom_command(TARGET lucaria_compiler POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_FILE_DIR:gltf2ozz>/gltf2ozz.exe"
		"$<TARGET_FILE_DIR:lucaria_compiler>/gltf2ozz.exe"
		MAIN_DEPENDENCY gltf2ozz)

	add_custom_command(TARGET lucaria_compiler POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_FILE_DIR:oggenc>/oggenc.exe"
		"$<TARGET_FILE_DIR:lucaria_compiler>/oggenc.exe"
		MAIN_DEPENDENCY oggenc)
endif()