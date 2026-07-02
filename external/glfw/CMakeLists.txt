set(glfw_main_sources
    "upstream/glfw/src/context.c"
    "upstream/glfw/src/init.c"
    "upstream/glfw/src/input.c"
    "upstream/glfw/src/monitor.c"
    "upstream/glfw/src/platform.c"
    "upstream/glfw/src/vulkan.c"
    "upstream/glfw/src/window.c"
    "upstream/glfw/src/egl_context.c"
    "upstream/glfw/src/osmesa_context.c"
    "upstream/glfw/src/null_init.c"
    "upstream/glfw/src/null_monitor.c"
    "upstream/glfw/src/null_window.c"
    "upstream/glfw/src/null_joystick.c")
if(APPLE)
    set(glfw_platform_sources
        "upstream/glfw/src/cocoa_time.c"
        "upstream/glfw/src/posix_module.c"
        "upstream/glfw/src/posix_thread.c")
elseif(WIN32)
    set(glfw_platform_sources
        "upstream/glfw/src/win32_time.c"
        "upstream/glfw/src/win32_module.c"
        "upstream/glfw/src/win32_thread.c"
        "upstream/glfw/src/win32_init.c"
        "upstream/glfw/src/win32_joystick.c"
        "upstream/glfw/src/win32_monitor.c"
        "upstream/glfw/src/win32_window.c"
        "upstream/glfw/src/wgl_context.c")    
else()
    set(glfw_platform_sources 
        "upstream/glfw/src/posix_module.c"
        "upstream/glfw/src/posix_poll.c"
        "upstream/glfw/src/posix_time.c"
        "upstream/glfw/src/posix_thread.c"
        "upstream/glfw/src/linux_joystick.c"
        "upstream/glfw/src/x11_init.c"
        "upstream/glfw/src/x11_monitor.c"
        "upstream/glfw/src/x11_window.c"
        "upstream/glfw/src/xkb_unicode.c"
        "upstream/glfw/src/glx_context.c")
endif()
set(glfw_sources 
    ${glfw_main_sources}
    ${glfw_platform_sources})

add_library(lucaria_external_glfw STATIC ${glfw_sources})
set_target_properties(lucaria_external_glfw PROPERTIES CXX_STANDARD 17)
target_include_directories(lucaria_external_glfw PUBLIC "upstream/glfw/include")
target_include_directories(lucaria_external_glfw PUBLIC "upstream/glfw/deps")

if(WIN32)
    target_compile_definitions(lucaria_external_glfw PRIVATE _GLFW_WIN32)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(lucaria_external_glfw PRIVATE _GLFW_X11)
    target_link_libraries(lucaria_external_glfw PUBLIC X11 Xrandr Xi Xcursor Xinerama)
endif()

if(MSVC)
    target_compile_options(lucaria_external_glfw PRIVATE /wd4034)
    target_compile_options(lucaria_external_glfw PRIVATE /wd4996)
endif()
