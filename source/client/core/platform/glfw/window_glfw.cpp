#include <lucaria/core/error.hpp>
#include <lucaria/core/window.hpp>

extern "C" void __lucaria_main_scene();

namespace lucaria {
namespace detail {

    namespace {

        static void _glfw_window_focus_callback(GLFWwindow* window, int focused)
        {
            window_implementation* _window = static_cast<window_implementation*>(glfwGetWindowUserPointer(window));

            if (focused) {
                _window->implementation_glfw.is_mouse_locked = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                _window->implementation_glfw.is_mouse_locked = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        static void _glfw_error_callback(int error, const char* description)
        {
            LUCARIA_RUNTIME_ERROR(std::string("GLFW Error: ") + description);
        }

        static void _glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            window_implementation* _window = static_cast<window_implementation*>(glfwGetWindowUserPointer(window));

            if (action == GLFW_PRESS) {
                if (key == GLFW_KEY_ESCAPE) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }

                if (key == GLFW_KEY_F11) {
                    if (_window->is_fullscreen) {
                        glfwSetWindowMonitor(window, nullptr, 50, 50, 1600, 900, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);

                    } else {
                        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
                            glfwGetVideoMode(glfwGetPrimaryMonitor())->height,
                            glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
                    }
                    _window->is_fullscreen = !_window->is_fullscreen;
                }

                if (glfw_keyboard_mappings.find(key) != glfw_keyboard_mappings.end()) {
                    if (!_window->implementation_glfw.is_mouse_locked) {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                        _window->implementation_glfw.is_mouse_locked = true;
                    }
                    _window->key_events[glfw_keyboard_mappings.at(key)].state = true;
                }

            } else if (action == GLFW_RELEASE) {
                if (glfw_keyboard_mappings.find(key) != glfw_keyboard_mappings.end()) {
                    _window->key_events[glfw_keyboard_mappings.at(key)].state = false;
                }
            }
        }

        static void _glfw_mouse_position_callback(GLFWwindow* window, const float64 xpos, const float64 ypos)
        {
            window_implementation* _window = static_cast<window_implementation*>(glfwGetWindowUserPointer(window));

            static float32x2 _last_position = float32x2(0);
            const float32x2 _new_position = float32x2(xpos, ypos);
            const float32x2 _delta_position = _new_position - _last_position;
            _last_position = _new_position;
            _window->pointer_accumulators[0] += _delta_position;
            _window->pointer_events[0].position = _new_position;
        }

        static void _glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
        {
            window_implementation* _window = static_cast<window_implementation*>(glfwGetWindowUserPointer(window));

            if (action == GLFW_PRESS) {
                switch (button) {
                case 0:
                    _window->key_events[input_key::mouse_left].state = true;
                    break;
                case 1:
                    _window->key_events[input_key::mouse_right].state = true;
                    break;
                case 2:
                    _window->key_events[input_key::mouse_middle].state = true;
                    break;
                default:
                    break;
                }

            } else if (action == GLFW_RELEASE) {
                switch (button) {
                case 0:
                    _window->key_events[input_key::mouse_left].state = false;
                    break;
                case 1:
                    _window->key_events[input_key::mouse_right].state = false;
                    break;
                case 2:
                    _window->key_events[input_key::mouse_middle].state = false;
                    break;
                default:
                    break;
                }
            }
        }

        void _initialize_backend(bool& is_s3tc_supported)
        {
            GLint _found_extensions_count = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &_found_extensions_count);
            for (GLint _extension_index = 0; _extension_index < _found_extensions_count; ++_extension_index) {
                const char* _extension_name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, _extension_index));
                if (std::string(_extension_name) == "GL_EXT_texture_compression_s3tc") {
                    is_s3tc_supported = true;
                }
            }
        }

        void _update_loop(window_implementation& window)
        {
            glfwPollEvents();

            static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
            const std::chrono::high_resolution_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
            window.time_delta_seconds = std::chrono::duration<float64>(_render_time - _last_render_time).count();
            _last_render_time = _render_time;

            int _screen_width, _screen_height;
            glfwGetFramebufferSize(window.implementation_glfw.window, &_screen_width, &_screen_height);
            window.screen_size = uint32x2(_screen_width, _screen_height);
            if (window.screen_size == uint32x2(0)) {
                return;
            }

            for (std::pair<const uint32, float32x2>& _accumulator : window.pointer_accumulators) {
                window.pointer_events[_accumulator.first].delta = _accumulator.second;
                _accumulator.second = float32x2(0);
            }

            ImGui_ImplGlfw_NewFrame();
            ImGui::GetIO().DisplaySize = convert_imgui(window.screen_size);

            window.stored_update_callback();

			glfwSwapBuffers(window.implementation_glfw.window);
        }
    }

    window_implementation::window_implementation(
		const std::function<void()>& update_callback, 
		const std::function<void()>& initialize_callback, 
		const std::function<void()>& destroy_callback
	)
    {
		set_engine_window(this);
		 
        glfwSetErrorCallback(_glfw_error_callback);
        if (!glfwInit()) {
            exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        implementation_glfw.window = glfwCreateWindow(1600, 900, "Lucaria", NULL, NULL);
        if (!implementation_glfw.window) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetWindowUserPointer(implementation_glfw.window, static_cast<void*>(this));
        glfwSetKeyCallback(implementation_glfw.window, _glfw_key_callback);
        glfwSetCursorPosCallback(implementation_glfw.window, _glfw_mouse_position_callback);
        glfwSetMouseButtonCallback(implementation_glfw.window, _glfw_mouse_button_callback);
        glfwMakeContextCurrent(implementation_glfw.window);
        glfwSetWindowFocusCallback(implementation_glfw.window, _glfw_window_focus_callback);

        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(0);

        _initialize_backend(is_s3tc_supported);
        initialize_imgui();
        initialize_openal();

        is_mouse_supported = true;
        is_keyboard_supported = true;
        is_touch_supported = false;
		
        if (initialize_callback) {
            initialize_callback();
        }
        stored_update_callback = update_callback;
		
		__lucaria_main_scene();

        while (!glfwWindowShouldClose(implementation_glfw.window)) {
            _update_loop(*this);
        }
        glfwDestroyWindow(implementation_glfw.window);
        glfwTerminate();

        if (destroy_callback) {
            destroy_callback();
        }
    }

}
}