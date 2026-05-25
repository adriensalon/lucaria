set(imgui_sources
	"upstream/imgui/imgui.cpp"
	"upstream/imgui/imgui_widgets.cpp"
	"upstream/imgui/imgui_tables.cpp"
	"upstream/imgui/imgui_draw.cpp"
	"upstream/imgui/imgui_demo.cpp"
	"upstream/imgui/misc/cpp/imgui_stdlib.cpp")

if(ANDROID)
    list(APPEND imgui_sources
		"upstream/imgui/backends/imgui_impl_android.cpp"
		"upstream/imgui/backends/imgui_impl_opengl3.cpp")		
elseif(EMSCRIPTEN)
    list(APPEND imgui_sources
		"upstream/imgui/backends/imgui_impl_opengl3.cpp")
elseif(WIN32)
    list(APPEND imgui_sources
		"upstream/imgui/backends/imgui_impl_glfw.cpp"
		"upstream/imgui/backends/imgui_impl_opengl3.cpp")
elseif(PSP)
    list(APPEND imgui_sources
		"upstream/imgui_psp/source/imgui_impl_psp.cpp")
endif()

add_library(lucaria_external_imgui STATIC EXCLUDE_FROM_ALL ${imgui_sources})
set_target_properties(lucaria_external_imgui PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_imgui PUBLIC "config")
target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui")
target_compile_definitions(lucaria_external_imgui PUBLIC -DIMGUI_USER_CONFIG="lucaria_imconfig.h")

if(PSP)
	target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui_psp/include")
endif()

if(ANDROID)
	target_link_libraries(lucaria_external_imgui PUBLIC GLESv3)
endif()
if(PSP)
	target_link_libraries(lucaria_external_imgui PUBLIC pspctrl pspgu psprtc)
endif()
if(WIN32)
	target_link_libraries(lucaria_external_imgui PRIVATE lucaria_external_glfw)
endif()