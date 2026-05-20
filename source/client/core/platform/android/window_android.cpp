#include <lucaria/core/error.hpp>
#include <lucaria/core/window.hpp>

extern "C" void __lucaria_main_scene();

namespace lucaria {
namespace detail {

    namespace {

        static void _redirect_stdio_to_log()
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

        static int32_t _android_on_input(android_app* app, AInputEvent* event)
        {
            // TODO: map touch / key events into your input system
            // You can start super simple and just ignore input for now:

            _is_mouse_locked = true;

            if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {

                static std::unordered_map<glm::uint, glm::vec2> _last_positions = {};
                for (int32_t _pointer_index = 0; _pointer_index < AMotionEvent_getPointerCount(event); _pointer_index++) {
                    glm::uint _event_id = static_cast<glm::uint>(AMotionEvent_getPointerId(event, _pointer_index));
                    glm::vec2& _last_position = _last_positions[_event_id];
                    const glm::vec2 _new_position = glm::vec2(AMotionEvent_getX(event, _pointer_index), AMotionEvent_getY(event, _pointer_index));
                    const glm::vec2 _delta_position = _new_position - _last_position;
                    _last_position = _new_position;
                    _pointer_accumulators[_event_id] += _delta_position;
                    _pointer_events[_event_id].position = _new_position;
                }
            }

            return ImGui_ImplAndroid_HandleInputEvent(event);
        }

        static void _android_on_app_cmd(android_app* app, int32_t cmd)
        {
            switch (cmd) {
            case APP_CMD_INIT_WINDOW:
                if (app->window != nullptr) {
                    lucaria::setup_opengl();
                    lucaria::setup_imgui();
                    lucaria::is_audio_locked = lucaria::setup_openal();
                    g_engine_initialized = true;
                }
                break;

            case APP_CMD_TERM_WINDOW:
                ImGui_ImplAndroid_Shutdown();
                destroy_opengl();
                destroy_openal();
                break;

            case APP_CMD_INPUT_CHANGED:
                std::cout << "APP_CMD_INPUT_CHANGED, inputQueue = " << app->inputQueue << "\n";
                break;

            case APP_CMD_DESTROY:
                destroy_opengl();
                destroy_openal();
                break;
            }
        }

        void _initialize_backend()
        {
            const EGLint config_attribs[] = {
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
            const EGLint context_attribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
            };
            g_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            eglInitialize(g_display, nullptr, nullptr);
            EGLConfig config;
            EGLint num_config;
            eglChooseConfig(g_display, config_attribs, &config, 1, &num_config);
            EGLint format;
            eglGetConfigAttrib(g_display, config, EGL_NATIVE_VISUAL_ID, &format);
            ANativeWindow_setBuffersGeometry(g_app->window, 0, 0, format);
            g_surface = eglCreateWindowSurface(g_display, config, g_app->window, nullptr);
            g_context = eglCreateContext(g_display, config, EGL_NO_CONTEXT, context_attribs);
            eglMakeCurrent(g_display, g_surface, g_surface, g_context);
            g_has_window = true;
        }

        void _destroy_backend()
        {
            if (g_display != EGL_NO_DISPLAY) {
                eglMakeCurrent(g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (g_context != EGL_NO_CONTEXT) {
                    eglDestroyContext(g_display, g_context);
                }
                if (g_surface != EGL_NO_SURFACE) {
                    eglDestroySurface(g_display, g_surface);
                }
                eglTerminate(g_display);
            }
            g_display = EGL_NO_DISPLAY;
            g_context = EGL_NO_CONTEXT;
            g_surface = EGL_NO_SURFACE;
            g_has_window = false;
        }

        void _update_loop(window_implementation& window)
        {
            static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
            const std::chrono::high_resolution_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
            time_delta_seconds = std::chrono::duration<glm::float64>(_render_time - _last_render_time).count();
            _last_render_time = _render_time;

            glm::int32 _screen_width, _screen_height;
            eglQuerySurface(g_display, g_surface, EGL_WIDTH, &_screen_width);
            eglQuerySurface(g_display, g_surface, EGL_HEIGHT, &_screen_height);
            screen_size = glm::uvec2(_screen_width, _screen_height);
            if (screen_size == glm::uvec2(0)) {
                return;
            }

            for (std::pair<const glm::uint, glm::vec2>& _accumulator : _pointer_accumulators) {
                _pointer_events[_accumulator.first].delta = _accumulator.second;
                _accumulator.second = glm::vec2(0);
            }

			ImGui_ImplAndroid_NewFrame();
			ImGui::GetIO().DisplaySize = ImVec2(static_cast<glm::float32>(screen_size.x), static_cast<glm::float32>(screen_size.y));

			window.stored_update_callback();

			eglSwapBuffers(window.display, window.surface);
        }
    }

    window_implementation::window_implementation(
		android_app* app, 			
		const std::function<void()>& update_callback, 
		const std::function<void()>& initialize_callback, 
		const std::function<void()>& destroy_callback
	)
    {
		set_engine_window(this);

        app_dummy();
        _redirect_stdio_to_log();

        implementation_android.app = app;
        implementation_android.app->onAppCmd = _android_on_app_cmd;
        implementation_android.app->onInputEvent = _android_on_input;

        is_mouse_supported = false;
        is_keyboard_supported = false;
        is_touch_supported = true;
        is_etc2_supported = true;
		
        if (initialize_callback) {
            initialize_callback();
        }
        stored_update_callback = update_callback;
		
		__lucaria_main_scene();

        while (true) {
            int _ident;
            int _events;
            android_poll_source* _poll_source = nullptr;

            while ((_ident = ALooper_pollOnce(has_window ? 0 : -1, nullptr, &_events, (void**)&_poll_source)) >= 0) {
                if (_poll_source) {
                    _poll_source->process(app, _poll_source);
                }

                if (implementation_android.app->destroyRequested) {
                    _destroy_backend();
                    destroy_openal();
                    return;
                }
            }

            if (is_engine_initialized && has_window && surface != EGL_NO_SURFACE) {
                _update_loop(*this);
            }
        }

        if (destroy_callback) {
            destroy_callback();
        }
    }
}
}