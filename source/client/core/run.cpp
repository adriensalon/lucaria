
#include <chrono>
#include <iostream>
#include <optional>
#include <thread>

#include <AL/al.h>
#include <AL/alc.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <lucaria/core/error.hpp>
#include <lucaria/core/input.hpp>
#include <lucaria/core/platform.hpp>
#include <lucaria/core/run.hpp>

#if LUCARIA_PLATFORM_ANDROID
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <backends/imgui_impl_android.h>
#include <unistd.h>
#elif LUCARIA_PLATFORM_WEB
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#elif LUCARIA_PLATFORM_WIN32
#define GLFW_INCLUDE_NONE
#define GLAD_GL_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <glad/gl.h>
#endif

#define _STRINGIFY(x) #x
#define _TO_STRING(x) _STRINGIFY(x)

namespace lucaria {

// export
bool _is_keyboard_supported = false;
bool _is_mouse_supported = false;
bool _is_touch_supported = false;
bool _is_etc2_supported = false;
bool _is_s3tc_supported = false;
std::vector<entt::registry>* global_scenes = nullptr;
std::unique_ptr<ImFontAtlas> _shared_font_atlas = nullptr;
glm::uint _shared_font_texture = 0;
ImGuiContext* _screen_context = nullptr;
ImGuiContext* _create_shared_context();
void _reupload_shared_font_texture();

// import
extern std::unordered_map<button_key, button_event> _button_events;
extern std::unordered_map<glm::uint, pointer_event> _pointer_events;
extern void _system_compute_motion();
extern void _system_compute_dynamics();
extern void _system_compute_mixer();
extern void _system_guizmos_motion();
extern void _system_guizmos_dynamics();
extern void _system_compute_rendering();

#if LUCARIA_PLATFORM_ANDROID
android_app* g_app = nullptr;
#elif LUCARIA_PLATFORM_WIN32
static GLFWwindow* glfw_window = nullptr;
#endif

namespace {

    static void setup_opengl();
    static bool setup_openal();
    static void setup_imgui();

    static void destroy_opengl();
    static void destroy_openal();
    static void destroy_imgui();

    static std::unordered_map<glm::uint, glm::vec2> _pointer_accumulators = {};
    static glm::uvec2 screen_size = { 0.f, 0.f };
    static glm::float64 time_delta_seconds = 0.f;
    static std::function<void()> update_callback = nullptr;
    static std::function<void()> teardown_callback = nullptr;

    static bool is_audio_locked = false;
    static bool _is_mouse_locked = false;
    static bool is_fullscreen = false;

#if LUCARIA_PLATFORM_WEB || LUCARIA_PLATFORM_ANDROID
    static std::unordered_map<std::string, button_key> emscripten_keyboard_mappings = {
        { "a", button_key::keyboard_a },
        { "z", button_key::keyboard_z },
        { "e", button_key::keyboard_e },
        { "r", button_key::keyboard_r },
        { "t", button_key::keyboard_t },
        { "y", button_key::keyboard_y },
        { "u", button_key::keyboard_u },
        { "i", button_key::keyboard_i },
        { "o", button_key::keyboard_o },
        { "p", button_key::keyboard_p },
        { "q", button_key::keyboard_q },
        { "s", button_key::keyboard_s },
        { "d", button_key::keyboard_d },
        { "f", button_key::keyboard_f },
        { "g", button_key::keyboard_g },
        { "h", button_key::keyboard_h },
        { "j", button_key::keyboard_j },
        { "k", button_key::keyboard_k },
        { "l", button_key::keyboard_l },
        { "m", button_key::keyboard_m },
        { "w", button_key::keyboard_w },
        { "x", button_key::keyboard_x },
        { "c", button_key::keyboard_c },
        { "v", button_key::keyboard_v },
        { "b", button_key::keyboard_b },
        { "n", button_key::keyboard_n },
    };
#elif LUCARIA_PLATFORM_WIN32
    static std::unordered_map<int, button_key> glfw_keyboard_mappings = {
        { GLFW_KEY_A, button_key::keyboard_a },
        { GLFW_KEY_Z, button_key::keyboard_z },
        { GLFW_KEY_E, button_key::keyboard_e },
        { GLFW_KEY_R, button_key::keyboard_r },
        { GLFW_KEY_T, button_key::keyboard_t },
        { GLFW_KEY_Y, button_key::keyboard_y },
        { GLFW_KEY_U, button_key::keyboard_u },
        { GLFW_KEY_I, button_key::keyboard_i },
        { GLFW_KEY_O, button_key::keyboard_o },
        { GLFW_KEY_P, button_key::keyboard_p },
        { GLFW_KEY_Q, button_key::keyboard_q },
        { GLFW_KEY_S, button_key::keyboard_s },
        { GLFW_KEY_D, button_key::keyboard_d },
        { GLFW_KEY_F, button_key::keyboard_f },
        { GLFW_KEY_G, button_key::keyboard_g },
        { GLFW_KEY_H, button_key::keyboard_h },
        { GLFW_KEY_J, button_key::keyboard_j },
        { GLFW_KEY_K, button_key::keyboard_k },
        { GLFW_KEY_L, button_key::keyboard_l },
        { GLFW_KEY_M, button_key::keyboard_m },
        { GLFW_KEY_W, button_key::keyboard_w },
        { GLFW_KEY_X, button_key::keyboard_x },
        { GLFW_KEY_C, button_key::keyboard_c },
        { GLFW_KEY_V, button_key::keyboard_v },
        { GLFW_KEY_B, button_key::keyboard_b },
        { GLFW_KEY_N, button_key::keyboard_n },
    };
#endif

#if LUCARIA_PLATFORM_WEB
    EM_JS(int, browser_get_samplerate, (), {
        var AudioContext = window.AudioContext || window.webkitAudioContext;
        var ctx = new AudioContext();
        var sr = ctx.sampleRate;
        ctx.close();
        return sr;
    });

    EM_JS(float, window_get_dpr, (), {
        return window.devicePixelRatio || 1.0;
    });

    EM_JS(int, canvas_get_width, (), {
        var _canvas = document.getElementById('canvas');
        var _dpr = window.devicePixelRatio;
        var _width = _canvas.getBoundingClientRect().width * _dpr;
        _canvas.width = _width;
        return _width;
    });

    EM_JS(int, canvas_get_height, (), {
        var _canvas = document.getElementById('canvas');
        var _dpr = window.devicePixelRatio;
        var _height = _canvas.getBoundingClientRect().height * _dpr;
        _canvas.height = _height;
        return _height;
    });

    EM_JS(int, navigator_get_touch_points, (), {
        return navigator.maxTouchPoints;
    });

    void emscripten_assert(EMSCRIPTEN_RESULT result)
    {
#if LUCARIA_CONFIG_DEBUG
        if (result != EMSCRIPTEN_RESULT_SUCCESS) {
            std::string _brief;
            bool _is_fatal = true;
            switch (result) {
            case EMSCRIPTEN_RESULT_DEFERRED:
                _brief = "EMSCRIPTEN_RESULT_DEFERRED";
                _is_fatal = false;
                break;
            case EMSCRIPTEN_RESULT_NOT_SUPPORTED:
                _brief = "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
                break;
            case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED:
                _brief = "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
                break;
            case EMSCRIPTEN_RESULT_INVALID_TARGET:
                _brief = "EMSCRIPTEN_RESULT_INVALID_TARGET";
                break;
            case EMSCRIPTEN_RESULT_UNKNOWN_TARGET:
                _brief = "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
                break;
            case EMSCRIPTEN_RESULT_INVALID_PARAM:
                _brief = "EMSCRIPTEN_RESULT_INVALID_PARAM";
                break;
            case EMSCRIPTEN_RESULT_FAILED:
                _brief = "EMSCRIPTEN_RESULT_FAILED";
                break;
            case EMSCRIPTEN_RESULT_NO_DATA:
                _brief = "EMSCRIPTEN_RESULT_NO_DATA";
                break;
            default:
                _brief = "Unknown emscripten result";
                break;
            }
            if (_is_fatal) {
                LUCARIA_RUNTIME_ERROR("Failed emscripten operation with result '" + _brief + "'")
            }
        }
#endif
    }

    EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent* event, void* user_data)
    {
        // process lock
        if (!is_audio_locked) {
            is_audio_locked = setup_openal();
        }
        EmscriptenPointerlockChangeEvent _pointer_lock;
        emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
        _is_mouse_locked = _pointer_lock.isActive;
        if (!_is_mouse_locked) {
            emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
            _is_mouse_locked = true;
        }

        const button_key _key(emscripten_keyboard_mappings[std::string(event->key)]);
        if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
            _button_events[_key].state = true;
        } else if (event_type == EMSCRIPTEN_EVENT_KEYUP) {
            _button_events[_key].state = false;
        }
        return 0;
    }

    EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent* event, void* user_data)
    {
        if (event_type == EMSCRIPTEN_EVENT_MOUSEMOVE) {
            const glm::float32 _dpr = window_get_dpr();
            const glm::vec2 _new_position = _dpr * glm::vec2(event->clientX, event->clientY);
            _pointer_accumulators[0] += _dpr * glm::vec2(event->movementX, event->movementY);
            _pointer_events[0].position = _new_position;
            ImGui::GetIO().AddMousePosEvent(_new_position.x, _new_position.y);
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) {
            const glm::uint _button = event->button;
            _button_events[static_cast<button_key>(_button)].state = true;
            ImGui::GetIO().AddMouseButtonEvent(_button, true);

            // process lock
            if (!is_audio_locked) {
                is_audio_locked = setup_openal();
            }
            EmscriptenPointerlockChangeEvent _pointer_lock;
            emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
            _is_mouse_locked = _pointer_lock.isActive;
            if (!_is_mouse_locked) {
                emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
                _is_mouse_locked = true;
            }
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
            const glm::uint _button = event->button;
            _button_events[static_cast<button_key>(_button)].state = false;
            ImGui::GetIO().AddMouseButtonEvent(_button, false);
        } else if (event_type == EMSCRIPTEN_EVENT_CLICK) {
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEOVER) {
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEOUT) {
        }
        return 0;
    }

    EM_BOOL touch_callback(int event_type, const EmscriptenTouchEvent* event, void* user_data)
    {
        // process lock
        if (!is_audio_locked) {
            is_audio_locked = setup_openal();
        }
        EmscriptenFullscreenChangeEvent _fullscreen;
        emscripten_assert(emscripten_get_fullscreen_status(&_fullscreen));
        _is_mouse_locked = _fullscreen.isFullscreen;
        if (!_is_mouse_locked) {
            EmscriptenFullscreenStrategy strategy;
            memset(&strategy, 0, sizeof(strategy));
            strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
            strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
            strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            strategy.canvasResizedCallback = [](int eventType,
                                                 const void* reserved,
                                                 void* userData) -> EM_BOOL { return EM_TRUE; };
            emscripten_assert(emscripten_request_fullscreen_strategy("#canvas", EM_TRUE, &strategy));
            _is_mouse_locked = true;
        }

        static std::unordered_map<glm::uint, glm::vec2> _last_positions = {};
        const float _dpr = window_get_dpr();
        if (event_type == EMSCRIPTEN_EVENT_TOUCHSTART) {
            for (int _pointer_index = 0; _pointer_index < event->numTouches; ++_pointer_index) {
                const EmscriptenTouchPoint& _touch_point = event->touches[_pointer_index];
                if (!_touch_point.isChanged) {
                    continue;
                }
                const glm::uint _event_id = static_cast<glm::uint>(_touch_point.identifier);
                const glm::vec2 _new_position = _dpr * glm::vec2(_touch_point.clientX, _touch_point.clientY);
                _last_positions[_event_id] = _new_position;
                _pointer_accumulators[_event_id] = glm::vec2(0);
                _pointer_events[_event_id].position = _new_position;
            }

        } else if (event_type == EMSCRIPTEN_EVENT_TOUCHMOVE) {
            for (int _pointer_index = 0; _pointer_index < event->numTouches; ++_pointer_index) {
                const EmscriptenTouchPoint& _touch_point = event->touches[_pointer_index];
                if (!_touch_point.isChanged) {
                    continue;
                }
                const glm::uint _event_id = static_cast<glm::uint>(_touch_point.identifier);
                const glm::vec2 _new_position = _dpr * glm::vec2(_touch_point.clientX, _touch_point.clientY);
                const glm::vec2 _delta_position = _new_position - _last_positions[_event_id];
                _last_positions[_event_id] = _new_position;
                _pointer_accumulators[_event_id] += _delta_position;
                _pointer_events[_event_id].position = _new_position;
            }

        } else if (event_type == EMSCRIPTEN_EVENT_TOUCHEND || event_type == EMSCRIPTEN_EVENT_TOUCHCANCEL) {
            for (int _pointer_index = 0; _pointer_index < event->numTouches; ++_pointer_index) {
                const EmscriptenTouchPoint& _touch_point = event->touches[_pointer_index];
                if (!_touch_point.isChanged) {
                    continue;
                }
                const glm::uint _event_id = static_cast<glm::uint>(_touch_point.identifier);
                _last_positions.erase(_event_id);
                _pointer_accumulators.erase(_event_id);
                _pointer_events.erase(_event_id);
            }
        }

        return 0; // we use preventDefault() for touch callbacks (see Safari on iPad)
    }
#endif

#if LUCARIA_PLATFORM_ANDROID
    static EGLDisplay g_display = EGL_NO_DISPLAY;
    static EGLSurface g_surface = EGL_NO_SURFACE;
    static EGLContext g_context = EGL_NO_CONTEXT;
    static bool g_has_window = false;
    static bool g_engine_initialized = false;

    void redirect_stdio_to_log()
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

    static int32_t android_on_input(android_app* app, AInputEvent* event)
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

    static void android_on_app_cmd(android_app* app, int32_t cmd)
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

#endif

#if LUCARIA_PLATFORM_WIN32

    static void glfw_window_focus_callback(GLFWwindow* window, int focused)
    {
        if (focused) {
            _is_mouse_locked = true;
            glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            _is_mouse_locked = false;
            glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    static void glfw_error_callback(int error, const char* description)
    {
        std::cout << "GLFW Error: " << description << std::endl;
    }

    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(glfw_window, GLFW_TRUE);
            }
            if (key == GLFW_KEY_F11) {
                if (is_fullscreen) {
                    glfwSetWindowMonitor(
                        glfw_window,
                        nullptr,
                        50,
                        50,
                        1600,
                        900,
                        glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
                } else {
                    glfwSetWindowMonitor(
                        glfw_window,
                        glfwGetPrimaryMonitor(),
                        0,
                        0,
                        glfwGetVideoMode(glfwGetPrimaryMonitor())->width,
                        glfwGetVideoMode(glfwGetPrimaryMonitor())->height,
                        glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
                }
                is_fullscreen = !is_fullscreen;
            }

            if (glfw_keyboard_mappings.find(key) != glfw_keyboard_mappings.end()) {
                if (!_is_mouse_locked) {
                    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    _is_mouse_locked = true;
                }
                _button_events[glfw_keyboard_mappings[key]].state = true;
            }
        } else if (action == GLFW_RELEASE) {
            if (glfw_keyboard_mappings.find(key) != glfw_keyboard_mappings.end()) {
                _button_events[glfw_keyboard_mappings[key]].state = false;
            }
        }
    }

    static void glfw_mouse_position_callback(GLFWwindow* window, const glm::float64 xpos, const glm::float64 ypos)
    {
        static glm::vec2 _last_position = glm::vec2(0);
        const glm::vec2 _new_position = glm::vec2(xpos, ypos);
        const glm::vec2 _delta_position = _new_position - _last_position;
        _last_position = _new_position;
        _pointer_accumulators[0] += _delta_position;
        _pointer_events[0].position = _new_position;
    }

    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (action == GLFW_PRESS) {
            _button_events[static_cast<button_key>(button)].state = true;
        } else if (action == GLFW_RELEASE) {
            _button_events[static_cast<button_key>(button)].state = false;
        }
    }

#endif

    void update_mouse_lock()
    {
#if LUCARIA_PLATFORM_WEB
        if (_is_touch_supported) {
            EmscriptenFullscreenChangeEvent _fullscreen;
            emscripten_assert(emscripten_get_fullscreen_status(&_fullscreen));
            _is_mouse_locked = _fullscreen.isFullscreen;

        } else {
            EmscriptenPointerlockChangeEvent _pointer_lock;
            emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
            _is_mouse_locked = _pointer_lock.isActive;
        }
#endif
    }

    static void setup_opengl()
    {
#if LUCARIA_PLATFORM_WEB
        EmscriptenWebGLContextAttributes _webgl_attributes;
        emscripten_webgl_init_context_attributes(&_webgl_attributes);
        _webgl_attributes.explicitSwapControl = 0;
        _webgl_attributes.depth = 1;
        _webgl_attributes.stencil = 1;
        _webgl_attributes.antialias = 1;
        _webgl_attributes.majorVersion = 2;
        _webgl_attributes.minorVersion = 0;
        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _webgl_context = emscripten_webgl_create_context("#canvas", &_webgl_attributes);
        if (_webgl_context < 0) {
            LUCARIA_RUNTIME_ERROR("Failed to create WebGL2 context on this device")
        }
        emscripten_assert(emscripten_webgl_make_context_current(_webgl_context));
        if (emscripten_webgl_enable_extension(_webgl_context, "WEBGL_compressed_texture_etc")) {
            _is_etc2_supported = true;
        }
        if (emscripten_webgl_enable_extension(_webgl_context, "WEBGL_compressed_texture_s3tc")) {
            _is_s3tc_supported = true;
        }
#endif

#if LUCARIA_PLATFORM_ANDROID
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
#endif

#if LUCARIA_PLATFORM_WIN32
        glm::int32 _found_extensions_count = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &_found_extensions_count);
        for (glm::int32 _extension_index = 0; _extension_index < _found_extensions_count; ++_extension_index) {
            const char* _extension_name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, _extension_index));
            if (std::string(_extension_name) == "GL_EXT_texture_compression_s3tc") {
                _is_s3tc_supported = true;
            }
        }
#endif
    }

    static void destroy_opengl()
    {
#if LUCARIA_PLATFORM_ANDROID
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
#endif
    }

    static void setup_imgui()
    {
        IMGUI_CHECKVERSION();
        _shared_font_atlas = std::make_unique<ImFontAtlas>();
        _shared_font_atlas->AddFontDefault();
        _reupload_shared_font_texture();
        _screen_context = _create_shared_context();
        ImGui::GetIO().IniFilename = NULL;
        ImGui::StyleColorsLight();
    }

    static bool setup_openal()
    {
        // https://emscripten.org/docs/porting/Audio.html
        // const ALCchar * devices = alcGetString( NULL, ALC_DEVICE_SPECIFIER );
        // std::cout << "Devices = " << std::string(devices) << std::endl;

        ALCdevice* _webaudio_device = alcOpenDevice(NULL);

        if (!_webaudio_device) {
            std::cout << "Impossible to create an OpenAL device" << std::endl;
            return false;
        }
        ALCcontext* _webaudio_context = alcCreateContext(_webaudio_device, NULL);
        if (!_webaudio_context) {
            std::cout << "Impossible to create an OpenAL context" << std::endl;
            return false;
        }
        if (!alcMakeContextCurrent(_webaudio_context)) {
            std::cout << "Impossible to use an OpenAL context" << std::endl;
            return false;
        }
        bool _is_float32_supported = (alIsExtensionPresent("AL_EXT_float32") == AL_TRUE);
        if (!_is_float32_supported) {
            std::cout << "OpenAL extension 'AL_EXT_float32' is not supported" << std::endl;
            return false;
        }
        return true;
    }

    static void destroy_openal()
    {
        ALCcontext* _webaudio_context = alcGetCurrentContext();
        ALCdevice* _webaudio_device = alcGetContextsDevice(_webaudio_context);
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(_webaudio_context);
        alcCloseDevice(_webaudio_device);
    }

    void update_loop()
    {
#if LUCARIA_PLATFORM_WIN32
        glfwPollEvents();
#endif

        // get time delta
#if LUCARIA_PLATFORM_WEB
        static glm::float64 _last_render_time = 0;
        const glm::float64 _render_time = emscripten_get_now();
        time_delta_seconds = (_render_time - _last_render_time) / 1000.f;
        _last_render_time = _render_time;
#else
        static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
        const std::chrono::steady_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
        time_delta_seconds = std::chrono::duration<glm::float64>(_render_time - _last_render_time).count();
        _last_render_time = _render_time;
#endif

        // get screen size
        glm::int32 _screen_width, _screen_height;
#if LUCARIA_PLATFORM_ANDROID
        eglQuerySurface(g_display, g_surface, EGL_WIDTH, &_screen_width);
        eglQuerySurface(g_display, g_surface, EGL_HEIGHT, &_screen_height);
#elif LUCARIA_PLATFORM_WEB
        _screen_width = canvas_get_width();
        _screen_height = canvas_get_height();
#elif LUCARIA_PLATFORM_WIN32
        glfwGetFramebufferSize(glfw_window, &_screen_width, &_screen_height);
#endif
        screen_size = glm::uvec2(_screen_width, _screen_height);
        if (screen_size == glm::uvec2(0)) {
            return;
        }

        // get delta pointers positions
        for (std::pair<const glm::uint, glm::vec2>& _accumulator : _pointer_accumulators) {
            _pointer_events[_accumulator.first].delta = _accumulator.second;
            _accumulator.second = glm::vec2(0);
        }

        // imgui platform backend new frame
#if LUCARIA_PLATFORM_ANDROID
        ImGui_ImplAndroid_NewFrame();
#elif LUCARIA_PLATFORM_WIN32
        ImGui_ImplGlfw_NewFrame();
#endif
        ImGui::GetIO().DisplaySize = ImVec2(static_cast<glm::float32>(screen_size.x), static_cast<glm::float32>(screen_size.y));

        // update
        update_callback();

        _system_compute_motion();
        _system_compute_dynamics();
        _system_compute_mixer();
        _system_guizmos_motion();
        _system_guizmos_dynamics();
        _system_compute_rendering();

        // swap buffers
#if LUCARIA_PLATFORM_ANDROID
        eglSwapBuffers(g_display, g_surface);
#elif LUCARIA_PLATFORM_WIN32
        glfwSwapBuffers(glfw_window);
#endif

        // assert
        // LUCARIA_RUNTIME_OPENGL_ASSERT
        if (is_audio_locked) {
            LUCARIA_RUNTIME_OPENAL_ASSERT
        }
    }

}

ImGuiContext* _create_shared_context()
{
    struct ImGui_ImplOpenGL3_Data {
        GLuint GlVersion;
        char GlslVersionString[32];
        bool GlProfileIsES2;
        bool GlProfileIsES3;
        bool GlProfileIsCompat;
        GLint GlProfileMask;
        GLuint FontTexture;
        GLuint ShaderHandle;
        GLint AttribLocationTex; // Uniforms location
        GLint AttribLocationProjMtx;
        GLuint AttribLocationVtxPos; // Vertex attributes location
        GLuint AttribLocationVtxUV;
        GLuint AttribLocationVtxColor;
        unsigned int VboHandle, ElementsHandle;
        GLsizeiptr VertexBufferSize;
        GLsizeiptr IndexBufferSize;
        bool HasClipOrigin;
        bool UseBufferSubData;

        ImGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
    };

    ImGuiContext* _context = ImGui::CreateContext(_shared_font_atlas.get());
    ImGui::SetCurrentContext(_context);

#if LUCARIA_PLATFORM_ANDROID
    static bool _must_install_callbacks = true;
    if (_must_install_callbacks) {
        ImGui_ImplAndroid_Init(g_app->window);
        _must_install_callbacks = false;
    }

#elif LUCARIA_PLATFORM_WIN32
    static bool _must_install_callbacks = true;
    if (_must_install_callbacks) {
        ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
        _must_install_callbacks = false;
    }
#endif

    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui::GetIO().Fonts->SetTexID((ImTextureID)(intptr_t)_shared_font_texture);
    ImGui::GetIO().IniFilename = nullptr;

    if (ImGui_ImplOpenGL3_Data* bd = static_cast<ImGui_ImplOpenGL3_Data*>(ImGui::GetIO().BackendRendererUserData)) {
        bd->FontTexture = _shared_font_texture;
    }

    return _context;
}

void _reupload_shared_font_texture()
{
    unsigned char* _pixels = nullptr;
    int _width, _height;
    _shared_font_atlas->GetTexDataAsRGBA32(&_pixels, &_width, &_height);

    if (_shared_font_texture == 0) {
        glGenTextures(1, &_shared_font_texture);
    }

    GLint _last_texture_handle = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &_last_texture_handle);
    glBindTexture(GL_TEXTURE_2D, _shared_font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels);
    glBindTexture(GL_TEXTURE_2D, _last_texture_handle);

    _shared_font_atlas->SetTexID((ImTextureID)(intptr_t)_shared_font_texture);
}

void set_update_callback(
    std::vector<entt::registry>& scenes,
    const std::function<void()>& on_update)
{
    global_scenes = &scenes;
    update_callback = on_update;
}

void set_teardown_callback(const std::function<void()>& on_teardown)
{
    teardown_callback = on_teardown;
}

glm::uvec2 get_screen_size()
{
    return screen_size;
}

glm::float64 get_time_delta()
{
    return time_delta_seconds;
}

bool get_is_etc2_supported()
{
    return _is_etc2_supported;
}

bool get_is_s3tc_supported()
{
    return _is_s3tc_supported;
}

bool get_is_game_locked()
{
    return is_audio_locked && _is_mouse_locked;
}

void each_scene(const std::function<void(entt::registry&)>& callback)
{
    if (global_scenes) {
        for (entt::registry& _scene : *global_scenes) {
            callback(_scene);
        }
    }
}
}

extern int lucaria_main(int argc, char** argv);

#if LUCARIA_PLATFORM_ANDROID
extern "C" void android_main(struct android_app* app)
#elif LUCARIA_PLATFORM_WIN32 && LUCARIA_HIDE_CONSOLE
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main(int argc, char** argv)
#endif
{
#if LUCARIA_PLATFORM_ANDROID
    app_dummy();
    lucaria::redirect_stdio_to_log();
#endif

    std::cout << "Built engine with generator: " << _TO_STRING(LUCARIA_GENERATOR) << std::endl;
    std::cout << "Built engine with compiler: " << _TO_STRING(LUCARIA_COMPILER) << std::endl;
    std::cout << "Built engine with config: " << _TO_STRING(LUCARIA_CONFIG) << std::endl;
    std::cout << "Built engine with simd: " << _TO_STRING(LUCARIA_SIMD) << std::endl;
    std::cout << "Built engine with packaging: " << (LUCARIA_OPTION_PACKAGE ? "ON" : "OFF") << std::endl;

#if LUCARIA_PLATFORM_ANDROID
    lucaria::g_app = app;
    lucaria::g_app->onAppCmd = lucaria::android_on_app_cmd;
    lucaria::g_app->onInputEvent = lucaria::android_on_input;
    lucaria::_is_mouse_supported = false;
    lucaria::_is_keyboard_supported = false;
    lucaria::_is_touch_supported = true;
    lucaria::_is_etc2_supported = true;
#endif

#if LUCARIA_PLATFORM_WEB
    lucaria::_is_touch_supported = lucaria::navigator_get_touch_points() > 1;
    lucaria::_is_mouse_supported = !lucaria::_is_touch_supported;
    lucaria::_is_keyboard_supported = !lucaria::_is_touch_supported;
    if (lucaria::_is_touch_supported) {
        lucaria::emscripten_assert(emscripten_set_touchstart_callback("#canvas", 0, 1, lucaria::touch_callback));
        lucaria::emscripten_assert(emscripten_set_touchend_callback("#canvas", 0, 1, lucaria::touch_callback));
        lucaria::emscripten_assert(emscripten_set_touchmove_callback("#canvas", 0, 1, lucaria::touch_callback));
        lucaria::emscripten_assert(emscripten_set_touchcancel_callback("#canvas", 0, 1, lucaria::touch_callback)); // EMSCRIPTEN_EVENT_TARGET_WINDOW doesnt seem to work on safari
    }
    if (lucaria::_is_mouse_supported) {
        lucaria::emscripten_assert(emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_mouseenter_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_mouseleave_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_mouseover_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
        lucaria::emscripten_assert(emscripten_set_mouseout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    }
    if (lucaria::_is_keyboard_supported) {
        lucaria::emscripten_assert(emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::key_callback));
        lucaria::emscripten_assert(emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::key_callback));
        lucaria::emscripten_assert(emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::key_callback));
    }

    lucaria::setup_opengl();
    lucaria::setup_imgui();
#endif

#if LUCARIA_PLATFORM_WIN32
    glfwSetErrorCallback(lucaria::glfw_error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    lucaria::glfw_window = glfwCreateWindow(1600, 900, "Lucaria", NULL, NULL);
    if (!lucaria::glfw_window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(lucaria::glfw_window, lucaria::glfw_key_callback);
    glfwSetCursorPosCallback(lucaria::glfw_window, lucaria::glfw_mouse_position_callback);
    glfwSetMouseButtonCallback(lucaria::glfw_window, lucaria::glfw_mouse_button_callback);
    glfwMakeContextCurrent(lucaria::glfw_window);
    glfwSetWindowFocusCallback(lucaria::glfw_window, lucaria::glfw_window_focus_callback);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    lucaria::setup_opengl();
    lucaria::setup_imgui();
    lucaria::is_audio_locked = lucaria::setup_openal();
    lucaria::_is_mouse_supported = true;
    lucaria::_is_keyboard_supported = true;
    lucaria::_is_touch_supported = false;
#endif

    std::cout << "Running engine with mouse: " << (lucaria::_is_mouse_supported ? "ON" : "OFF") << std::endl;
    std::cout << "Running engine with keyboard: " << (lucaria::_is_keyboard_supported ? "ON" : "OFF") << std::endl;
    std::cout << "Running engine with touch: " << (lucaria::_is_touch_supported ? "ON" : "OFF") << std::endl;
    std::cout << "Running engine with compression: " << (lucaria::_is_etc2_supported ? "ETC2" : (lucaria::_is_s3tc_supported ? "S3TC" : "OFF")) << std::endl;
    // audio float32

    // register game code
#if LUCARIA_PLATFORM_ANDROID
    lucaria_main(0, nullptr);
#elif LUCARIA_PLATFORM_WIN32
    lucaria_main(__argc, __argv);
#else
    lucaria_main(argc, argv);
#endif

#if LUCARIA_PLATFORM_WEB
    emscripten_set_main_loop(lucaria::update_loop, 0, EM_TRUE);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

#elif LUCARIA_PLATFORM_ANDROID
    while (true) {
        int ident;
        int events;
        android_poll_source* source = nullptr;
        while ((ident = ALooper_pollOnce(g_has_window ? 0 : -1, nullptr, &events, (void**)&source)) >= 0) {
            if (source) {
                source->process(g_app, source);
            }
            if (g_app->destroyRequested) {
                lucaria::destroy_opengl();
                lucaria::destroy_openal();
                return;
            }
        }
        if (lucaria::g_engine_initialized && lucaria::g_has_window && lucaria::g_surface != EGL_NO_SURFACE) {
            lucaria::update_loop();
        }
    }

#elif LUCARIA_PLATFORM_WIN32
    while (!glfwWindowShouldClose(lucaria::glfw_window)) {
        lucaria::update_loop();
    }
    glfwDestroyWindow(lucaria::glfw_window);
    glfwTerminate();
#endif

    if (lucaria::teardown_callback) {
        lucaria::teardown_callback();
    }
}
