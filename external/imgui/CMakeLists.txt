set(imgui_sources
	"upstream/imgui/imgui.cpp"
	"upstream/imgui/imgui_widgets.cpp"
	"upstream/imgui/imgui_tables.cpp"
	"upstream/imgui/imgui_draw.cpp"
	"upstream/imgui/imgui_demo.cpp"
	"upstream/imgui/misc/cpp/imgui_stdlib.cpp"	
	"upstream/imgui/backends/imgui_impl_opengl3.cpp")

if(ANDROID)
	set(imgui_sources
		${imgui_sources}
		"upstream/imgui/backends/imgui_impl_android.cpp")
elseif(WIN32)
	set(imgui_sources
		${imgui_sources}
		"upstream/imgui/backends/imgui_impl_glfw.cpp")
endif()

add_library(lucaria_external_imgui STATIC EXCLUDE_FROM_ALL ${imgui_sources})
set_target_properties(lucaria_external_imgui PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui")
target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui/backends")

if(ANDROID)
	target_link_libraries(lucaria_external_imgui PUBLIC GLESv3)
endif()
if(WIN32)
	target_link_libraries(lucaria_external_imgui PRIVATE lucaria_external_glfw)
endif()