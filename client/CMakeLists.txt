cmake_minimum_required(VERSION 3.12)
project(lucaria)

# cmake_policy(SET CMP0077 NEW)
# cmake_policy(SET CMP0115 NEW)

function(message)
	if(NOT MESSAGE_QUIET)
		_message(${ARGN})
	endif()
endfunction()

set(MESSAGE_QUIET ON)
add_subdirectory(../external/ogg ${CMAKE_BINARY_DIR}/external/ogg)
set(OGG_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/ogg/include)
set(OGG_LIBRARIES ogg)

add_subdirectory(../external/entt ${CMAKE_BINARY_DIR}/external/entt)
add_subdirectory(../external/glm ${CMAKE_BINARY_DIR}/external/glm)

option(INSTALL_CMAKE_PACKAGE_MODULE OFF)
add_subdirectory(../external/vorbis ${CMAKE_BINARY_DIR}/external/vorbis)
set(VORBIS_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/../external/vorbis/include)

add_subdirectory(../external/imgui ${CMAKE_BINARY_DIR}/external/imgui)
target_compile_options(imgui PRIVATE -Wno-format-security)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_BULLET3 OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_UNIT_TESTS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BULLET_LINK_STATIC ON)
option(BULLET_USE_OPENCL OFF)
add_subdirectory(../external/bullet ${CMAKE_BINARY_DIR}/external/bullet)
target_compile_options(BulletSoftBody PRIVATE -Wno-single-bit-bitfield-constant-conversion)
target_compile_options(BulletSoftBody PRIVATE -Wno-implicit-const-int-float-conversion)
set(BULLET_INCLUDE_DIRS 
	${CMAKE_CURRENT_LIST_DIR}/../external/bullet/src
	${CMAKE_CURRENT_LIST_DIR}/../external/bullet/src/BulletCollision
	${CMAKE_CURRENT_LIST_DIR}/../external/bullet/src/BulletDynamics
	${CMAKE_CURRENT_LIST_DIR}/../external/bullet/src/LinearMath)

set(JUST_INSTALL_CEREAL ON)
set(SKIP_PERFORMANCE_COMPARISON ON)
add_subdirectory(../external/cereal ${CMAKE_BINARY_DIR}/external/cereal)

set(ozz_build_tools OFF)
set(ozz_build_fbx OFF)
set(ozz_build_gltf OFF)
set(ozz_build_data OFF)
set(ozz_build_postfix OFF)
set(ozz_build_samples OFF)
set(ozz_build_howtos OFF)
set(ozz_run_tests_headless OFF)
add_subdirectory(../external/ozz ${CMAKE_BINARY_DIR}/external/ozz)
target_compile_options(ozz_base PUBLIC "-Wno-#pragma-messages")
target_compile_options(ozz_base PUBLIC "-Wno-suggest-override")
target_compile_options(ozz_base PUBLIC "-Wno-float-equal")
target_compile_options(ozz_base PUBLIC "-Wno-newline-eof")
target_compile_options(ozz_base PUBLIC "-Wno-deprecated-declarations")
target_compile_options(ozz_base PUBLIC "-Wno-suggest-override")
target_compile_options(ozz_base PUBLIC "-Wno-suggest-destructor-override")
target_compile_options(ozz_base PUBLIC "-Wno-exit-time-destructors")
target_compile_options(ozz_base PUBLIC "-Wno-global-constructors")
target_compile_options(ozz_base PUBLIC "-Wno-sign-conversion")
target_compile_options(ozz_base PUBLIC "-Wno-reserved-id-macro")
target_compile_options(ozz_base PUBLIC "-Wno-covered-switch-default")

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(../external/woff ${CMAKE_BINARY_DIR}/external/woff)
set(MESSAGE_QUIET OFF)

file(GLOB_RECURSE lucaria_sources "${CMAKE_CURRENT_LIST_DIR}/source/*.cpp")
add_executable(lucaria ${lucaria_sources})

target_compile_options(lucaria PRIVATE -Wno-format-security)

target_include_directories(lucaria PRIVATE ${BULLET_INCLUDE_DIRS})
target_include_directories(lucaria PRIVATE ${OGG_INCLUDE_DIRS})
target_include_directories(lucaria PRIVATE ${VORBIS_INCLUDE_DIRS})

target_include_directories(lucaria PRIVATE ${CMAKE_CURRENT_LIST_DIR}/source)
target_include_directories(lucaria PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../common/include)

target_link_libraries(lucaria PRIVATE cereal)
target_link_libraries(lucaria PRIVATE EnTT)
target_link_libraries(lucaria PRIVATE glm)
target_link_libraries(lucaria PRIVATE imgui)
target_link_libraries(lucaria PRIVATE ogg)
target_link_libraries(lucaria PRIVATE vorbis vorbisfile)
target_link_libraries(lucaria PRIVATE ozz_animation)
target_link_libraries(lucaria PRIVATE woff2dec)
target_link_libraries(lucaria PRIVATE BulletDynamics BulletCollision LinearMath)

# target_compile_definitions(lucaria PRIVATE LUCARIA_DEBUG)
target_compile_definitions(lucaria PRIVATE LUCARIA_GUIZMO)
# target_compile_definitions(lucaria PRIVATE LUCARIA_JSON)

set_target_properties(lucaria PROPERTIES CXX_STANDARD 17)

if(DEFINED CMAKE_TOOLCHAIN_FILE AND CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten")
	set(CXX_STANDARD ".html")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--use-preload-cache")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
		-g \
		--shell-file ${CMAKE_CURRENT_LIST_DIR}/source/shell.html \
		-s LLD_REPORT_UNDEFINED \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s MAX_WEBGL_VERSION=2 \
		-s USE_WEBGL2=1 \
		-s GL_DEBUG=1 \
		-s FETCH \
		-s NO_DISABLE_EXCEPTION_CATCHING \
		-lopenal")
	# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
	# 	-O3 \
	# 	--shell-file ${CMAKE_CURRENT_LIST_DIR}/source/shell.html \
	# 	-s LLD_REPORT_UNDEFINED \
	# 	-s ALLOW_MEMORY_GROWTH=1 \
	# 	-s MAX_WEBGL_VERSION=2 \
	# 	-s USE_WEBGL2=1 \
	# 	-s FETCH \
	# 	-lopenal")

# -O3 \
# --preload-file ${CMAKE_CURRENT_LIST_DIR}/../install/assets@/ \
else()
	set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
	set(GLFW_LIBRARY_TYPE STATIC)
	add_subdirectory(../external/glfw ${CMAKE_BINARY_DIR}/external/glfw)
	target_include_directories(lucaria PRIVATE ${GLFW_SOURCE_DIR}/deps)
	target_link_libraries(lucaria PRIVATE glfw)

	set(FORCE_STATIC_VCRT ON)
	set(LIBTYPE STATIC)
	add_subdirectory(../external/openalsoft ${CMAKE_BINARY_DIR}/external/openalsoft)
	target_link_libraries(lucaria PRIVATE OpenAL)
	# message(FATAL_ERROR "CMake is being configured by host platform but we require Emscripten, aborting...")
	# return()

	add_custom_command(
		TARGET lucaria
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/client/lucaria.exe ${INSTALL_DIR}/client/lucaria.exe
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/client/lucaria.pdb ${INSTALL_DIR}/client/lucaria.pdb)
endif()
