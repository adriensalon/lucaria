#include <chrono>
#include <thread>
#include <unordered_map>

#include <tracy/Tracy.hpp>

#include <lucaria/core/manager_app.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_input.hpp>
#include <lucaria/core/rendering_vulkan.hpp>
#include <lucaria/core/utils_math.hpp>

namespace lucaria {
namespace detail {
    namespace {

        struct android_window_context {
            manager_window* window = nullptr;
            manager_input* input = nullptr;
            manager_assets* assets = nullptr;
        };

        void _redirect_stdio_to_log()
        {
            int _pfd[2];
            pipe(_pfd);
            dup2(_pfd[1], STDOUT_FILENO);
            dup2(_pfd[1], STDERR_FILENO);
            std::thread([=]() {
                char _buf[256];
                ssize_t _r;
                while ((_r = read(_pfd[0], _buf, sizeof(_buf) - 1)) > 0) {
                    _buf[_r] = 0;
                    __android_log_write(ANDROID_LOG_INFO, "lucaria", _buf);
                }
            }).detach();
        }

        int32_t _android_on_input(android_app* app, AInputEvent* event)
        {
            android_window_context* _context = static_cast<android_window_context*>(app->userData);
            if (_context == nullptr || _context->input == nullptr || _context->window == nullptr) {
                return ImGui_ImplAndroid_HandleInputEvent(event);
            }

            if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
                static std::unordered_map<uint32, float32x2> _last_positions = {};
                const int32_t _pointer_count = AMotionEvent_getPointerCount(event);
                for (int32_t _pointer_index = 0; _pointer_index < _pointer_count; ++_pointer_index) {
                    const uint32 _event_id = static_cast<uint32>(AMotionEvent_getPointerId(event, _pointer_index));
                    float32x2& _last_position = _last_positions[_event_id];
                    const float32x2 _new_position = float32x2(
                        AMotionEvent_getX(event, _pointer_index),
                        AMotionEvent_getY(event, _pointer_index));
                    const float32x2 _delta_position = _new_position - _last_position;
                    _last_position = _new_position;
                    _context->window->pointer_accumulators[_event_id] += _delta_position;
                    _context->input->pointer_events[_event_id].position = _new_position;
                }
            }

            return ImGui_ImplAndroid_HandleInputEvent(event);
        }

#if defined(LUCARIA_BACKEND_OPENGL)
        void _initialize_opengl(manager_window& window)
        {
            const EGLint _config_attribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_STENCIL_SIZE, 8,
                EGL_NONE
            };
            const EGLint _context_attribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
            };
            window.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            eglInitialize(window.display, nullptr, nullptr);
            EGLConfig _config;
            EGLint _num_config;
            eglChooseConfig(window.display, _config_attribs, &_config, 1, &_num_config);
            EGLint _format;
            eglGetConfigAttrib(window.display, _config, EGL_NATIVE_VISUAL_ID, &_format);
            ANativeWindow_setBuffersGeometry(window.app->window, 0, 0, _format);
            window.surface = eglCreateWindowSurface(window.display, _config, window.app->window, nullptr);
            window.context = eglCreateContext(window.display, _config, EGL_NO_CONTEXT, _context_attribs);
            eglMakeCurrent(window.display, window.surface, window.surface, window.context);
        }

        void _destroy_opengl(manager_window& window)
        {
            if (window.display != EGL_NO_DISPLAY) {
                eglMakeCurrent(window.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (window.context != EGL_NO_CONTEXT) {
                    eglDestroyContext(window.display, window.context);
                }
                if (window.surface != EGL_NO_SURFACE) {
                    eglDestroySurface(window.display, window.surface);
                }
                eglTerminate(window.display);
            }
            window.display = EGL_NO_DISPLAY;
            window.context = EGL_NO_CONTEXT;
            window.surface = EGL_NO_SURFACE;
        }
#endif

        void _initialize_backend(manager_window& window, manager_assets& assets)
        {
#if defined(LUCARIA_BACKEND_OPENGL)
            _initialize_opengl(window);
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
            rendering_vulkan_initialize(window.app->window);
#endif
            window.has_window = true;
            assets.is_etc2_supported = true;
        }

        void _destroy_backend(manager_window& window)
        {
#if defined(LUCARIA_BACKEND_VULKAN)
            rendering_vulkan_shutdown();
#endif
#if defined(LUCARIA_BACKEND_OPENGL)
            _destroy_opengl(window);
#endif
            window.has_window = false;
            window.is_engine_initialized = false;
        }

        void _android_on_app_cmd(android_app* app, int32_t cmd)
        {
            android_window_context* _context = static_cast<android_window_context*>(app->userData);
            if (_context == nullptr || _context->window == nullptr || _context->assets == nullptr) {
                return;
            }
            manager_window& _window = *_context->window;
            switch (cmd) {
            case APP_CMD_INIT_WINDOW:
                if (app->window != nullptr) {
                    _initialize_backend(_window, *_context->assets);
                    _window.initialize_imgui();
                    _window.initialize_openal();
                    _window.is_engine_initialized = true;
                }
                break;
            case APP_CMD_TERM_WINDOW:
                _window.destroy_imgui();
                _window.destroy_openal();
                _destroy_backend(_window);
                break;
            case APP_CMD_DESTROY:
                _destroy_backend(_window);
                break;
            default:
                break;
            }
        }

        void _update_screen_size(manager_window& window)
        {
#if defined(LUCARIA_BACKEND_OPENGL)
            int32 _screen_width = 0;
            int32 _screen_height = 0;
            eglQuerySurface(window.display, window.surface, EGL_WIDTH, &_screen_width);
            eglQuerySurface(window.display, window.surface, EGL_HEIGHT, &_screen_height);
            window.screen_size = uint32x2(_screen_width, _screen_height);
#endif
#if defined(LUCARIA_BACKEND_VULKAN)
            window.screen_size = uint32x2(
                static_cast<uint32>(ANativeWindow_getWidth(window.app->window)),
                static_cast<uint32>(ANativeWindow_getHeight(window.app->window)));
#endif
        }

        void _update_loop(manager_window& window, manager_input& input)
        {
            static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
            const std::chrono::high_resolution_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
            window.time_delta_seconds = std::chrono::duration<float64>(_render_time - _last_render_time).count();
            _last_render_time = _render_time;

            _update_screen_size(window);
            if (window.screen_size == uint32x2(0)) {
                return;
            }

            for (std::pair<const uint32, float32x2>& _accumulator : window.pointer_accumulators) {
                input.pointer_events[_accumulator.first].delta = _accumulator.second;
                _accumulator.second = float32x2(0);
            }

            ImGui_ImplAndroid_NewFrame();
            ImGui::GetIO().DisplaySize = convert_imgui(window.screen_size);

#if defined(LUCARIA_BACKEND_VULKAN)
            rendering_vulkan_begin_frame(window.screen_size);
#endif
            window.stored_update_callback();
#if defined(LUCARIA_BACKEND_VULKAN)
            rendering_vulkan_end_frame();
#endif

#if defined(LUCARIA_BACKEND_OPENGL)
            eglSwapBuffers(window.display, window.surface);
#endif
        }

    }

    void manager_window::run(
        android_app* app,
        manager_input& input,
        manager_assets& objects,
        const std::function<void()>& setup_callback,
        const std::function<void()>& update_callback)
    {
        tracy::SetThreadName("Main Thread");
        app_dummy();
        _redirect_stdio_to_log();

        this->app = app;
        android_window_context _context = { this, &input, &objects };
        app->userData = &_context;
        app->onAppCmd = _android_on_app_cmd;
        app->onInputEvent = _android_on_input;

        input.is_mouse_supported = false;
        input.is_keyboard_supported = false;
        input.is_touch_supported = true;
        objects.is_etc2_supported = true;

        setup_callback();
        stored_update_callback = update_callback;

        while (true) {
            int _ident;
            int _events;
            android_poll_source* _poll_source = nullptr;
            while ((_ident = ALooper_pollOnce(has_window ? 0 : -1, nullptr, &_events, reinterpret_cast<void**>(&_poll_source))) >= 0) {
                if (_poll_source != nullptr) {
                    _poll_source->process(app, _poll_source);
                }
                if (app->destroyRequested) {
                    _destroy_backend(*this);
                    return;
                }
            }
            if (is_engine_initialized && has_window) {
                ZoneScopedN("Frame");
                _update_loop(*this, input);
            }
        }
    }

}
}
