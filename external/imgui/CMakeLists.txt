if(NOT LUCARIA_BUILD_COMPILERS)

	set(imgui_sources
		"upstream/imgui_1897/imgui.cpp"
		"upstream/imgui_1897/imgui_widgets.cpp"
		"upstream/imgui_1897/imgui_tables.cpp"
		"upstream/imgui_1897/imgui_draw.cpp"
		"upstream/imgui_1897/imgui_demo.cpp"
		"upstream/imgui_1897/misc/cpp/imgui_stdlib.cpp")
	if(ANDROID)
		list(APPEND imgui_sources
			"upstream/imgui_1897/backends/imgui_impl_android.cpp"
			"upstream/imgui_1897/backends/imgui_impl_opengl3.cpp")		
	elseif(EMSCRIPTEN)
		list(APPEND imgui_sources
			"upstream/imgui_1897/backends/imgui_impl_opengl3.cpp")
	elseif(WIN32)
		list(APPEND imgui_sources
			"upstream/imgui_1897/backends/imgui_impl_glfw.cpp"
			"upstream/imgui_1897/backends/imgui_impl_opengl3.cpp")
	elseif(PSP)
		list(APPEND imgui_sources
			"upstream/imgui_psp/source/imgui_impl_psp.cpp")
	endif()	

	add_library(lucaria_external_imgui STATIC EXCLUDE_FROM_ALL ${imgui_sources})
	set_target_properties(lucaria_external_imgui PROPERTIES CXX_STANDARD 17)
	target_include_directories(lucaria_external_imgui PUBLIC "config")
	target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui_1897")
	target_compile_definitions(lucaria_external_imgui PUBLIC -DIMGUI_USER_CONFIG="lucaria_imconfig.h")

	if(ANDROID)
		target_link_libraries(lucaria_external_imgui PUBLIC GLESv3)
	endif()
	if(PSP)
		target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui_psp/include")
		target_link_libraries(lucaria_external_imgui PUBLIC pspctrl pspgu psprtc)
	endif()
	if(WIN32)
		target_link_libraries(lucaria_external_imgui PRIVATE lucaria_external_glfw)
	endif()

	# interface
	add_library(lucaria_external_imgui_interface INTERFACE)
	target_include_directories(lucaria_external_imgui_interface INTERFACE "config")
	target_include_directories(lucaria_external_imgui_interface INTERFACE "upstream/imgui_1897")
	target_compile_definitions(lucaria_external_imgui_interface INTERFACE -DIMGUI_USER_CONFIG="lucaria_imconfig.h")
	if(WIN32 AND NOT LUCARIA_DISABLE_RELOAD)
		target_compile_definitions(lucaria_external_imgui PRIVATE
			"IMGUI_API=__declspec(dllexport)")
	endif()

else()

	set(lucaria_patched_imgui_dir "${CMAKE_CURRENT_BINARY_DIR}/patched")
	set(lucaria_patched_imgui_dir "${lucaria_patched_imgui_dir}" PARENT_SCOPE)

	file(MAKE_DIRECTORY "${lucaria_patched_imgui_dir}/backends")
	file(READ "upstream/imgui_1927/backends/imgui_impl_opengl3_loader.h" IMGUI_GL_LOADER_TEXT)

	# needed by Tracy texture code
	string(REPLACE
		"#define GL_LINEAR                         0x2601"
		"#define GL_LINEAR                         0x2601\n#define GL_LINEAR_MIPMAP_LINEAR          0x2703"
		IMGUI_GL_LOADER_TEXT
		"${IMGUI_GL_LOADER_TEXT}")

	# add the missing typedef
	string(REPLACE
		"typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);"
		"typedef void (APIENTRYP PFNGLTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);\ntypedef void (APIENTRYP PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);"
		IMGUI_GL_LOADER_TEXT
		"${IMGUI_GL_LOADER_TEXT}")

	# increase proc pointer array
	string(REPLACE
		"GL3WglProc ptr[63];"
		"GL3WglProc ptr[64];"
		IMGUI_GL_LOADER_TEXT
		"${IMGUI_GL_LOADER_TEXT}")

	# append to struct at the end, so existing function pointer order is unchanged
	string(REPLACE
		"PFNGLVIEWPORTPROC                 Viewport;"
		"PFNGLVIEWPORTPROC                 Viewport;\n        PFNGLCOMPRESSEDTEXIMAGE2DPROC     CompressedTexImage2D;"
		IMGUI_GL_LOADER_TEXT
		"${IMGUI_GL_LOADER_TEXT}")

	# add macro after glViewport
	string(REPLACE
		"#define glViewport                        imgl3wProcs.gl.Viewport"
		"#define glViewport                        imgl3wProcs.gl.Viewport\n#define glCompressedTexImage2D             imgl3wProcs.gl.CompressedTexImage2D"
		IMGUI_GL_LOADER_TEXT
		"${IMGUI_GL_LOADER_TEXT}")

	# append to proc_names[] at the same position as the struct append
	string(REPLACE
		"    \"glViewport\",\n};"
		"    \"glViewport\",\n    \"glCompressedTexImage2D\",\n};"
		IMGUI_GL_LOADER_TEXT
		"${IMGUI_GL_LOADER_TEXT}")

	if(NOT IMGUI_GL_LOADER_TEXT MATCHES "GL3WglProc ptr\\[64\\];")
		message(FATAL_ERROR "Failed to resize ImGL3WProcs ptr array")
	endif()

	if(NOT IMGUI_GL_LOADER_TEXT MATCHES "PFNGLCOMPRESSEDTEXIMAGE2DPROC[ \t]+CompressedTexImage2D;")
		message(FATAL_ERROR "Failed to add CompressedTexImage2D to ImGL3WProcs")
	endif()

	if(NOT IMGUI_GL_LOADER_TEXT MATCHES "#define glCompressedTexImage2D[ \t]+imgl3wProcs\\.gl\\.CompressedTexImage2D")
		message(FATAL_ERROR "Failed to add glCompressedTexImage2D macro")
	endif()

	if(NOT IMGUI_GL_LOADER_TEXT MATCHES "\"glCompressedTexImage2D\"")
		message(FATAL_ERROR "Failed to add glCompressedTexImage2D to proc_names")
	endif()

	file(WRITE "${lucaria_patched_imgui_dir}/backends/imgui_impl_opengl3_loader.h" "${IMGUI_GL_LOADER_TEXT}")

	configure_file(
		"upstream/imgui_1927/backends/imgui_impl_opengl3.cpp"
		"${lucaria_patched_imgui_dir}/backends/imgui_impl_opengl3.cpp"
		COPYONLY)
	configure_file(
		"upstream/imgui_1927/backends/imgui_impl_opengl3.h"
		"${lucaria_patched_imgui_dir}/backends/imgui_impl_opengl3.h"
		COPYONLY)

	set(imgui_sources
		"upstream/imgui_1927/imgui.cpp"
		"upstream/imgui_1927/imgui_widgets.cpp"
		"upstream/imgui_1927/imgui_tables.cpp"
		"upstream/imgui_1927/imgui_draw.cpp"
		"upstream/imgui_1927/imgui_demo.cpp"
		"upstream/imgui_1927/misc/cpp/imgui_stdlib.cpp")
	list(APPEND imgui_sources
		"upstream/imgui_1927/backends/imgui_impl_glfw.cpp"
		"${lucaria_patched_imgui_dir}/backends/imgui_impl_opengl3.cpp")

	add_library(lucaria_external_imgui STATIC EXCLUDE_FROM_ALL ${imgui_sources})
	set_target_properties(lucaria_external_imgui PROPERTIES CXX_STANDARD 17)
	target_include_directories(lucaria_external_imgui BEFORE PUBLIC "${lucaria_patched_imgui_dir}")
	target_include_directories(lucaria_external_imgui PUBLIC "config")
	target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui_1927")
	target_compile_definitions(lucaria_external_imgui PUBLIC -DIMGUI_USER_CONFIG="lucaria_imconfig.h")
	target_link_libraries(lucaria_external_imgui PRIVATE lucaria_external_glfw)

endif()
