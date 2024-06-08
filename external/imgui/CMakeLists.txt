cmake_minimum_required(VERSION 3.0.0)
project(imgui)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic")

set(SOURCE_IMGUI
	"imgui.cpp"
	"imgui_widgets.cpp"
	"imgui_tables.cpp"
	"imgui_draw.cpp"
	"imgui_demo.cpp"

	"backends/imgui_impl_opengl3.cpp"

	# "implot/implot.cpp"
	# "implot/implot_items.cpp"
	# "implot/implot_demo.cpp"

	# "imnodes/imnodes.cpp"

	# "imguizmo/imguizmo.cpp"
)

# includes
include_directories("backends/")
include_directories("../sdl/include/")

# include_directories("../glad/opengl/windows")

# build
add_library(imgui STATIC ${SOURCE_IMGUI})

# link
# if (${CMAKE_SYSTEM_NAME} MATCHES "Android")
# target_link_libraries(imgui opengl_android)
# elseif (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
# target_link_libraries(imgui opengl_windows)
# endif ()

# properties
set_property(TARGET imgui PROPERTY CXX_STANDARD 17)

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

# set_property(TARGET imgui PROPERTY ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Windows)

# add_custom_command(TARGET imgui COMMAND python EMakeLists.py WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
