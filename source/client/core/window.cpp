
#include <chrono>
#include <iostream>
#include <optional>

#include <AL/al.h>
#include <AL/alc.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#elif defined(__ANDROID__)
#include <android_native_app_glue.h>
#include <backends/imgui_impl_android.h>
#else
#include <backends/imgui_impl_glfw.h>
#endif

#include <lucaria/core/animation.hpp>
#include <lucaria/core/cubemap.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/font.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/opengl.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/shape.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/core/sound.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>

#define _STRINGIFY(x) #x
#define _TO_STRING(x) _STRINGIFY(x)

namespace lucaria {
struct ImGui_ImplOpenGL3_Data {
    GLuint GlVersion; // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
    char GlslVersionString[32]; // Specified by user or detected based on compile time GL settings.
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

namespace {

    static void setup_opengl();
    static void destroy_opengl();
    static bool setup_openal();
    static void setup_imgui();

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
    static std::unordered_map<std::string, keyboard_key> emscripten_keyboard_mappings = {
        { "a", keyboard_key::a },
        { "z", keyboard_key::z },
        { "e", keyboard_key::e },
        { "r", keyboard_key::r },
        { "t", keyboard_key::t },
        { "y", keyboard_key::y },
        { "u", keyboard_key::u },
        { "i", keyboard_key::i },
        { "o", keyboard_key::o },
        { "p", keyboard_key::p },
        { "q", keyboard_key::q },
        { "s", keyboard_key::s },
        { "d", keyboard_key::d },
        { "f", keyboard_key::f },
        { "g", keyboard_key::g },
        { "h", keyboard_key::h },
        { "j", keyboard_key::j },
        { "k", keyboard_key::k },
        { "l", keyboard_key::l },
        { "m", keyboard_key::m },
        { "w", keyboard_key::w },
        { "x", keyboard_key::x },
        { "c", keyboard_key::c },
        { "v", keyboard_key::v },
        { "b", keyboard_key::b },
        { "n", keyboard_key::n },
    };
#else
    static GLFWwindow* glfw_window = nullptr;
    static std::unordered_map<int, keyboard_key> glfw_keyboard_mappings = {
        { GLFW_KEY_A, keyboard_key::a },
        { GLFW_KEY_Z, keyboard_key::z },
        { GLFW_KEY_E, keyboard_key::e },
        { GLFW_KEY_R, keyboard_key::r },
        { GLFW_KEY_T, keyboard_key::t },
        { GLFW_KEY_Y, keyboard_key::y },
        { GLFW_KEY_U, keyboard_key::u },
        { GLFW_KEY_I, keyboard_key::i },
        { GLFW_KEY_O, keyboard_key::o },
        { GLFW_KEY_P, keyboard_key::p },
        { GLFW_KEY_Q, keyboard_key::q },
        { GLFW_KEY_S, keyboard_key::s },
        { GLFW_KEY_D, keyboard_key::d },
        { GLFW_KEY_F, keyboard_key::f },
        { GLFW_KEY_G, keyboard_key::g },
        { GLFW_KEY_H, keyboard_key::h },
        { GLFW_KEY_J, keyboard_key::j },
        { GLFW_KEY_K, keyboard_key::k },
        { GLFW_KEY_L, keyboard_key::l },
        { GLFW_KEY_M, keyboard_key::m },
        { GLFW_KEY_W, keyboard_key::w },
        { GLFW_KEY_X, keyboard_key::x },
        { GLFW_KEY_C, keyboard_key::c },
        { GLFW_KEY_V, keyboard_key::v },
        { GLFW_KEY_B, keyboard_key::b },
        { GLFW_KEY_N, keyboard_key::n },
    };
#endif

    static std::unordered_map<keyboard_key, bool> keys = {};
    static std::vector<keyboard_key> keys_changed = {};
    static std::unordered_map<glm::uint, bool> buttons = {};
    static std::vector<glm::uint> buttons_changed = {};
    static glm::uvec2 screen_size = { 0.f, 0.f };
    static glm::vec2 mouse_position = { 0.f, 0.f };
    static glm::vec2 mouse_position_delta = { 0.f, 0.f };
    static glm::vec2 accumulated_mouse_position_delta = { 0.f, 0.f };
    static glm::float64 time_delta_seconds = 0.f;
    static std::function<void()> update_callback = nullptr;

    static bool is_multitouch_supported = false;
    static bool is_etc2_supported = false;
    static bool is_s3tc_supported = false;
    static bool is_audio_locked = false;
    static bool is_mouse_locked = false;
    static bool is_fullscreen = false;

#if defined(__EMSCRIPTEN__)
    EM_JS(int, browser_get_samplerate, (), {
        var AudioContext = window.AudioContext || window.webkitAudioContext;
        var ctx = new AudioContext();
        var sr = ctx.sampleRate;
        ctx.close();
        return sr;
    });

    EM_JS(int, canvas_get_width, (), {
        var canvas = document.getElementById('canvas');
        canvas.width = canvas.getBoundingClientRect().width;
        return canvas.getBoundingClientRect().width;
    });

    EM_JS(int, canvas_get_height, (), {
        var canvas = document.getElementById('canvas');
        canvas.height = canvas.getBoundingClientRect().height;
        return canvas.getBoundingClientRect().height;
    });

    EM_JS(int, navigator_get_multitouch, (), {
        if (navigator.maxTouchPoints && navigator.maxTouchPoints >= 2) {
            return 1;
        }
        if (navigator.msMaxTouchPoints && navigator.msMaxTouchPoints >= 2) {
            return 1;
        }
        if ("ontouchstart" in window) {
            return 1;
        }
        return 0;
    });

    void emscripten_assert(EMSCRIPTEN_RESULT result)
    {
#if LUCARIA_DEBUG
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

    void process_lock()
    {
        if (!is_audio_locked) {
            is_audio_locked = setup_openal();
        }
        if (!is_mouse_locked) {
            emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
            is_mouse_locked = true;
        }
    }

    EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent* event, void* user_data)
    {
        const keyboard_key _key(emscripten_keyboard_mappings[std::string(event->key)]);
        if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
            keys[_key] = true;
            keys_changed.emplace_back(_key);
            process_lock();
        } else if (event_type == EMSCRIPTEN_EVENT_KEYUP) {
            keys[_key] = false;
            keys_changed.emplace_back(_key);
        }
        return 0;
    }

    EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent* event, void* user_data)
    {
        if (event_type == EMSCRIPTEN_EVENT_MOUSEMOVE) {
            accumulated_mouse_position_delta += glm::vec2(event->movementX, event->movementY);
            // std::cout << "x = " << event->screenX << ", y = " << event->screenY << "ok" << std::endl;
            // detail::mouse_position_delta = glm::vec2(event->clientX, event->clientY) - detail::mouse_position;
            mouse_position = glm::vec2(event->clientX, event->clientY);
            ImGui::GetIO().AddMousePosEvent(mouse_position.x, mouse_position.y);
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) {
            glm::uint _button = event->button;
            buttons[_button] = true;
            buttons_changed.emplace_back(_button);
            ImGui::GetIO().AddMouseButtonEvent(_button, true);
            process_lock();
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
            glm::uint _button = event->button;
            buttons[_button] = false;
            buttons_changed.emplace_back(_button);
            ImGui::GetIO().AddMouseButtonEvent(_button, false);
        } else if (event_type == EMSCRIPTEN_EVENT_CLICK) {
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEOVER) {
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEOUT) {
        }
        return 0;
    }

    EM_BOOL touch_callback(int event_type, const EmscriptenTouchEvent* event, void* user_data)
    {
        const EmscriptenTouchPoint* _touch_point = &(event->touches[0]);
        glm::vec2 _touch_move_position = glm::vec2(_touch_point->clientX, _touch_point->clientY);
        if (event_type == EMSCRIPTEN_EVENT_TOUCHSTART) {
            process_lock();
            mouse_position_delta = { 0, 0 };
        } else if (event_type == EMSCRIPTEN_EVENT_TOUCHMOVE)
            mouse_position_delta = _touch_move_position - mouse_position;
        mouse_position = _touch_move_position;
        return EM_TRUE; // we use preventDefault() for touch callbacks (see Safari on iPad)
    }
#endif

#if defined(__ANDROID__)
    static android_app* g_app = nullptr;
    static EGLDisplay g_display = EGL_NO_DISPLAY;
    static EGLSurface g_surface = EGL_NO_SURFACE;
    static EGLContext g_context = EGL_NO_CONTEXT;
    static bool g_has_window = false;
    static bool g_engine_initialized = false;

    static int32_t android_on_input(android_app* app, AInputEvent* event)
    {
        // TODO: map touch / key events into your input system
        // You can start super simple and just ignore input for now:
        return 0;
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

        case APP_CMD_DESTROY:
            destroy_opengl();
            destroy_openal();
            break;
        }
    }

#endif

#if defined(_WIN32)

    static void glfw_window_focus_callback(GLFWwindow* window, int focused)
    {
        if (focused) {
            is_mouse_locked = true;
            std::cout << "Window gained focus\n";
            glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            is_mouse_locked = false;
            std::cout << "Window lost focus\n";
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
                if (!is_mouse_locked) {
                    glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    is_mouse_locked = true;
                }
                keys[glfw_keyboard_mappings[key]] = true;
            }
        } else if (action == GLFW_RELEASE) {
            if (glfw_keyboard_mappings.find(key) != glfw_keyboard_mappings.end()) {
                keys[glfw_keyboard_mappings[key]] = false;
            }
        }
    }

    static void glfw_mouse_position_callback(GLFWwindow* window, const glm::float64 xpos, const glm::float64 ypos)
    {
        static glm::float64 _last_x, _last_y;
        const glm::float64 _delta_x = xpos - _last_x;
        const glm::float64 _delta_y = ypos - _last_y;
        _last_x = xpos;
        _last_y = ypos;
        accumulated_mouse_position_delta += glm::vec2(_delta_x, _delta_y);
    }

    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (action == GLFW_PRESS) {
            buttons[static_cast<glm::uint>(button)] = true;
        } else if (action == GLFW_RELEASE) {
            buttons[static_cast<glm::uint>(button)] = false;
        }
    }

#endif

    void update_mouse_lock()
    {
#if defined(__EMSCRIPTEN__)
        EmscriptenPointerlockChangeEvent _pointer_lock;
        emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
        is_mouse_locked = _pointer_lock.isActive;
#else

#endif
    }

    static void setup_opengl()
    {
#if defined(__EMSCRIPTEN__)
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
            is_etc2_supported = true;
        }
        if (emscripten_webgl_enable_extension(_webgl_context, "WEBGL_compressed_texture_s3tc")) {
            is_s3tc_supported = true;
        }

#elif defined(__ANDROID__)
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

#elif defined(_WIN32)
        glm::int32 _found_extensions_count = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &_found_extensions_count);
        for (glm::int32 _extension_index = 0; _extension_index < _found_extensions_count; ++_extension_index) {
            const char* _extension_name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, _extension_index));
            if (std::string(_extension_name) == "GL_EXT_texture_compression_etc") {
                is_etc2_supported = true;
            }
            if (std::string(_extension_name) == "GL_EXT_texture_compression_s3tc") {
                is_s3tc_supported = true;
            }
        }
#endif
    }

    static void destroy_opengl()
    {
#if defined(__ANDROID__)
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
        detail::global_imgui_shared_font_atlas = std::make_unique<ImFontAtlas>();
        detail::global_imgui_shared_font_atlas->AddFontDefault();
        detail::reupload_shared_font_texture_RGBA32();
        detail::global_imgui_screen_context = detail::create_shared_context();
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
        // get time delta
#if defined(__EMSCRIPTEN__)
        static glm::float64 _last_render_time = 0;
        const glm::float64 _render_time = emscripten_get_now();
        time_delta_seconds = (_render_time - _last_render_time) / 1000.f;
#else
        static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
        const std::chrono::steady_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
        time_delta_seconds = std::chrono::duration<glm::float64>(_render_time - _last_render_time).count();
#endif

        // get screen size
        glm::int32 _screen_width, _screen_height;
#if defined(__ANDROID__)
        eglQuerySurface(g_display, g_surface, EGL_WIDTH, &_screen_width);
        eglQuerySurface(g_display, g_surface, EGL_HEIGHT, &_screen_height);
#elif defined(__EMSCRIPTEN__)
        _screen_width = canvas_get_width();
        _screen_height = canvas_get_height();
#elif defined(_WIN32)
        glfwGetFramebufferSize(glfw_window, &_screen_width, &_screen_height);
#endif
        screen_size = glm::uvec2(_screen_width, _screen_height);
        if (screen_size == glm::uvec2(0)) {
            return;
        }

        // get mouse delta
        static glm::vec2 _last_accum_pos_delta = glm::vec2(0, 0);
        mouse_position_delta = accumulated_mouse_position_delta - _last_accum_pos_delta;
        _last_accum_pos_delta = accumulated_mouse_position_delta;
        _last_render_time = _render_time;

        // imgui platform backend new frame
#if defined(__ANDROID__)
        ImGui_ImplAndroid_NewFrame();
#elif defined(_WIN32)
        ImGui_ImplGlfw_NewFrame();
#endif

        // update
        ImGui::GetIO().DisplaySize = ImVec2(static_cast<glm::float32>(screen_size.x), static_cast<glm::float32>(screen_size.y));
        update_mouse_lock();
        update_callback();
        keys_changed.clear();
        buttons_changed.clear();

        // swap buffers
#if defined(__ANDROID__)
        eglSwapBuffers(g_display, g_surface);
#elif defined(_WIN32)
        glfwSwapBuffers(glfw_window);
        glfwPollEvents();
#endif

        // assert
        LUCARIA_RUNTIME_OPENGL_ASSERT
        if (is_audio_locked) {
            LUCARIA_RUNTIME_OPENAL_ASSERT
        }
    }

}

namespace detail {

    void run_game(const std::function<void()>& update)
    {
        update_callback = update;

#if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(update_loop, 0, EM_TRUE);
        emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);

#elif defined(__ANDROID__)
        while (true) {
            int ident;
            int events;
            android_poll_source* source = nullptr;
            while ((ident = ALooper_pollOnce(g_has_window ? 0 : -1, nullptr, &events, (void**)&source)) >= 0) {
                if (source) {
                    source->process(g_app, source);
                }
                if (g_app->destroyRequested) {
                    destroy_opengl();
                    destroy_openal();
                    return;
                }
            }
            if (g_engine_initialized && g_has_window && g_surface != EGL_NO_SURFACE) {
                update_loop();
            }
        }
        
#elif defined(_WIN32)
        while (!glfwWindowShouldClose(glfw_window)) {
            update_loop();
        }
        glfwDestroyWindow(glfw_window);
        glfwTerminate();
#endif

        destroy_scenes();
    }

    ImGuiContext* create_shared_context()
    {
        ImGuiContext* _context = ImGui::CreateContext(global_imgui_shared_font_atlas.get());
        ImGui::SetCurrentContext(_context);

#if defined(__ANDROID__)
        static bool _must_install_callbacks = true;
        if (_must_install_callbacks) {
            ImGui_ImplAndroid_Init(g_app->window);
            _must_install_callbacks = false;
        }

#elif defined(_WIN32)
        static bool _must_install_callbacks = true;
        if (_must_install_callbacks) {
            ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
            _must_install_callbacks = false;
        }
#endif

        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui::GetIO().Fonts->SetTexID((ImTextureID)(intptr_t)global_imgui_shared_font_texture);
        ImGui::GetIO().IniFilename = nullptr;

        if (ImGui_ImplOpenGL3_Data* bd = static_cast<ImGui_ImplOpenGL3_Data*>(ImGui::GetIO().BackendRendererUserData)) {
            bd->FontTexture = global_imgui_shared_font_texture;
        }

        return _context;
    }

    void reupload_shared_font_texture_RGBA32()
    {
        unsigned char* _pixels = nullptr;
        int _width, _height;
        global_imgui_shared_font_atlas->GetTexDataAsRGBA32(&_pixels, &_width, &_height);

        if (global_imgui_shared_font_texture == 0) {
            glGenTextures(1, &global_imgui_shared_font_texture);
        }

        GLint _last_texture_handle = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &_last_texture_handle);
        glBindTexture(GL_TEXTURE_2D, global_imgui_shared_font_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels);
        glBindTexture(GL_TEXTURE_2D, _last_texture_handle);

        global_imgui_shared_font_atlas->SetTexID((ImTextureID)(intptr_t)global_imgui_shared_font_texture);
    }
}

std::unordered_map<keyboard_key, bool>& get_keys()
{
    return keys;
}

std::unordered_map<glm::uint, bool>& get_buttons()
{
    return buttons;
}

glm::uvec2 get_screen_size()
{
    return screen_size;
}

glm::vec2 get_mouse_position()
{
    return mouse_position;
}

glm::vec2& get_mouse_position_delta()
{
    return mouse_position_delta;
}

glm::float64 get_time_delta()
{
    return time_delta_seconds;
}

bool get_is_multitouch_supported()
{
    return is_multitouch_supported;
}

bool get_is_etc2_supported()
{
    return is_etc2_supported;
}

bool get_is_s3tc_supported()
{
    return is_s3tc_supported;
}

bool get_is_audio_locked()
{
    return is_audio_locked;
}

bool get_is_mouse_locked()
{
    return is_mouse_locked;
}
}

extern int lucaria_main();

#if defined(__ANDROID__)
extern "C" void android_main(struct android_app* app)
#elif defined(_WIN32) && LUCARIA_HIDE_CONSOLE
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
    std::cout << "Built engine with generator: " << _TO_STRING(LUCARIA_GENERATOR) << std::endl;
    std::cout << "Built engine with compiler: " << _TO_STRING(LUCARIA_COMPILER) << std::endl;
    std::cout << "Built engine with config: " << _TO_STRING(LUCARIA_CONFIG) << std::endl;
    std::cout << "Built engine with simd: " << _TO_STRING(LUCARIA_SIMD) << std::endl;
    std::cout << "Built engine with exceptions: " << (LUCARIA_DEBUG ? "ON (Select config other than Debug to disable)" : "OFF (Select Debug config to enable)") << std::endl;
    std::cout << "Built engine with guizmos: " << (LUCARIA_GUIZMO ? "ON (Select config other than Debug to disable)" : "OFF (Select Debug config to enable)") << std::endl;

#if defined(__ANDROID__)
    lucaria::g_app = app;
    lucaria::g_app->onAppCmd = lucaria::android_on_app_cmd;
    lucaria::g_app->onInputEvent = lucaria::android_on_input;
    lucaria::is_multitouch_supported = true;

#elif defined(__EMSCRIPTEN__)
    lucaria::emscripten_assert(emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::key_callback));
    lucaria::emscripten_assert(emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::key_callback));
    lucaria::emscripten_assert(emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::key_callback));
    lucaria::emscripten_assert(emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_mouseenter_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_mouseleave_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_mouseover_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_mouseout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, lucaria::mouse_callback));
    lucaria::emscripten_assert(emscripten_set_touchstart_callback("#canvas", 0, 1, lucaria::touch_callback));
    lucaria::emscripten_assert(emscripten_set_touchend_callback("#canvas", 0, 1, lucaria::touch_callback));
    lucaria::emscripten_assert(emscripten_set_touchmove_callback("#canvas", 0, 1, lucaria::touch_callback));
    lucaria::emscripten_assert(emscripten_set_touchcancel_callback("#canvas", 0, 1, lucaria::touch_callback)); // EMSCRIPTEN_EVENT_TARGET_WINDOW doesnt seem to work on safari
    lucaria::setup_opengl();
    lucaria::setup_imgui();
    lucaria::is_audio_locked = lucaria::setup_openal();
    lucaria::is_multitouch_supported = lucaria::navigator_get_multitouch();

#elif defined(_WIN32)
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
    lucaria::is_multitouch_supported = false;
#endif

    std::cout << "Running engine with multitouch: " << (lucaria::is_multitouch_supported ? "ON" : "OFF") << std::endl;
    std::cout << "Running engine with compression: " << (lucaria::is_etc2_supported ? "ETC2" : (lucaria::is_s3tc_supported ? "S3TC" : "NONE")) << std::endl;

    lucaria_main();
}
