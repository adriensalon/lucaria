set(imgui_sources
	"upstream/imgui/imgui.cpp"
	"upstream/imgui/imgui_widgets.cpp"
	"upstream/imgui/imgui_tables.cpp"
	"upstream/imgui/imgui_draw.cpp"
	"upstream/imgui/imgui_demo.cpp"
	"upstream/imgui/misc/cpp/imgui_stdlib.cpp"
	# "upstream/imgui/backends/imgui_impl_glfw.cpp"
	"upstream/imgui/backends/imgui_impl_opengl3.cpp")

add_library(lucaria_external_imgui STATIC EXCLUDE_FROM_ALL ${imgui_sources})
set_target_properties(lucaria_external_imgui PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui")
target_include_directories(lucaria_external_imgui PUBLIC "upstream/imgui/backends")
