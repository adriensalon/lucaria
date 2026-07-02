#include <tracy/Tracy.hpp>

#include <lucaria/core/manager_app.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_input.hpp>
#include <lucaria/core/utils_math.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static const std::unordered_map<int, input_key> _glfw_keyboard_mappings = {
            { GLFW_KEY_A, input_key::keyboard_a },
            { GLFW_KEY_Z, input_key::keyboard_z },
            { GLFW_KEY_E, input_key::keyboard_e },
            { GLFW_KEY_R, input_key::keyboard_r },
            { GLFW_KEY_T, input_key::keyboard_t },
            { GLFW_KEY_Y, input_key::keyboard_y },
            { GLFW_KEY_U, input_key::keyboard_u },
            { GLFW_KEY_I, input_key::keyboard_i },
            { GLFW_KEY_O, input_key::keyboard_o },
            { GLFW_KEY_P, input_key::keyboard_p },
            { GLFW_KEY_Q, input_key::keyboard_q },
            { GLFW_KEY_S, input_key::keyboard_s },
            { GLFW_KEY_D, input_key::keyboard_d },
            { GLFW_KEY_F, input_key::keyboard_f },
            { GLFW_KEY_G, input_key::keyboard_g },
            { GLFW_KEY_H, input_key::keyboard_h },
            { GLFW_KEY_J, input_key::keyboard_j },
            { GLFW_KEY_K, input_key::keyboard_k },
            { GLFW_KEY_L, input_key::keyboard_l },
            { GLFW_KEY_M, input_key::keyboard_m },
            { GLFW_KEY_W, input_key::keyboard_w },
            { GLFW_KEY_X, input_key::keyboard_x },
            { GLFW_KEY_C, input_key::keyboard_c },
            { GLFW_KEY_V, input_key::keyboard_v },
            { GLFW_KEY_B, input_key::keyboard_b },
            { GLFW_KEY_N, input_key::keyboard_n },
        };

        struct _glfw_context {
            manager_window* window = nullptr;
            manager_input* input = nullptr;
        };

        static void _glfw_window_focus_callback(GLFWwindow* window, int focused)
        {
            _glfw_context* _context = static_cast<_glfw_context*>(glfwGetWindowUserPointer(window));

            if (focused) {
                _context->window->is_mouse_locked = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                _context->window->is_mouse_locked = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        static void _glfw_error_callback(int error, const char* description)
        {
            LUCARIA_DEBUG_ERROR(std::string("GLFW Error: ") + description);
        }

        static void _glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            _glfw_context* _context = static_cast<_glfw_context*>(glfwGetWindowUserPointer(window));

            if (action == GLFW_PRESS) {
                if (key == GLFW_KEY_ESCAPE) {
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                }

                if (key == GLFW_KEY_F11) {
                    if (_context->window->is_fullscreen) {
                        glfwSetWindowMonitor(window, nullptr, 50, 50, 1600, 900, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);

                    } else {
                        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
                            glfwGetVideoMode(glfwGetPrimaryMonitor())->height,
                            glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
                    }
                    _context->window->is_fullscreen = !_context->window->is_fullscreen;
                }

                if (_glfw_keyboard_mappings.find(key) != _glfw_keyboard_mappings.end()) {
                    if (!_context->window->is_mouse_locked) {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                        _context->window->is_mouse_locked = true;
                    }
                    _context->input->key_events[_glfw_keyboard_mappings.at(key)].state = true;
                }

            } else if (action == GLFW_RELEASE) {
                if (_glfw_keyboard_mappings.find(key) != _glfw_keyboard_mappings.end()) {
                    _context->input->key_events[_glfw_keyboard_mappings.at(key)].state = false;
                }
            }
        }

        static void _glfw_mouse_position_callback(GLFWwindow* window, const float64 xpos, const float64 ypos)
        {
            _glfw_context* _context = static_cast<_glfw_context*>(glfwGetWindowUserPointer(window));

            static float32x2 _last_position = float32x2(0);
            const float32x2 _new_position = float32x2(xpos, ypos);
            const float32x2 _delta_position = _new_position - _last_position;
            _last_position = _new_position;
            _context->window->pointer_accumulators[0] += _delta_position;
            _context->input->pointer_events[0].position = _new_position;
        }

        static void _glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
        {
            _glfw_context* _context = static_cast<_glfw_context*>(glfwGetWindowUserPointer(window));

            if (action == GLFW_PRESS) {
                switch (button) {
                case 0:
                    _context->input->key_events[input_key::mouse_left].state = true;
                    break;
                case 1:
                    _context->input->key_events[input_key::mouse_right].state = true;
                    break;
                case 2:
                    _context->input->key_events[input_key::mouse_middle].state = true;
                    break;
                default:
                    break;
                }

            } else if (action == GLFW_RELEASE) {
                switch (button) {
                case 0:
                    _context->input->key_events[input_key::mouse_left].state = false;
                    break;
                case 1:
                    _context->input->key_events[input_key::mouse_right].state = false;
                    break;
                case 2:
                    _context->input->key_events[input_key::mouse_middle].state = false;
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

        void _update_loop(manager_window& window, manager_input& input)
        {
            glfwPollEvents();

            static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
            const std::chrono::high_resolution_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
            window.time_delta_seconds = std::chrono::duration<float64>(_render_time - _last_render_time).count();
            _last_render_time = _render_time;

            int _screen_width, _screen_height;
            glfwGetFramebufferSize(window.window, &_screen_width, &_screen_height);
            window.screen_size = uint32x2(_screen_width, _screen_height);
            if (window.screen_size == uint32x2(0)) {
                return;
            }

            for (std::pair<const uint32, float32x2>& _accumulator : window.pointer_accumulators) {
                const float32x2 _delta = _accumulator.second;
                input.pointer_events[_accumulator.first].delta = _delta;
                if (_accumulator.first == 0) {
                    input.mouse_position_delta = _delta;
                    input.mouse_position = input.pointer_events[0].position;
                }
                _accumulator.second = float32x2(0);
            }

            ImGui_ImplGlfw_NewFrame();
            ImGui::GetIO().DisplaySize = convert_imgui(window.screen_size);

            window.stored_update_callback();

            glfwSwapBuffers(window.window);
        }
    }

    void manager_window::run(
        manager_input& input,
        manager_assets& objects,
        const std::function<void()>& setup_callback,
        const std::function<void()>& update_callback)
    {
        tracy::SetThreadName("Main Thread");

        input.is_mouse_supported = true;
        input.is_keyboard_supported = true;
        input.is_touch_supported = false;

        glfwSetErrorCallback(_glfw_error_callback);
        if (!glfwInit()) {
            exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(1600, 900, "Lucaria", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        _glfw_context _user_pointer = { this, &input };
        glfwSetWindowUserPointer(window, static_cast<void*>(&_user_pointer));
        glfwSetKeyCallback(window, _glfw_key_callback);
        glfwSetCursorPosCallback(window, _glfw_mouse_position_callback);
        glfwSetMouseButtonCallback(window, _glfw_mouse_button_callback);
        glfwSetWindowFocusCallback(window, _glfw_window_focus_callback);

        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(0);

        _initialize_backend(objects.is_s3tc_supported);
        initialize_imgui();
        initialize_openal();

        setup_callback();
        stored_update_callback = update_callback;
        while (!glfwWindowShouldClose(window)) {
            ZoneScopedN("Frame");
            _update_loop(*this, input);
        }
        glfwDestroyWindow(window);
        glfwTerminate();
    }

}
}