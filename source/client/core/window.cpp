
#include <iostream>
#include <optional>

#include <AL/al.h>
#include <AL/alc.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#else
#include <backends/imgui_impl_glfw.h>
#include <chrono>
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

static bool _use_imgui = false;
static bool _use_imgui_command = false;
void use_imgui_rendering(const bool use)
{
    _use_imgui_command = use;
}

namespace detail {

    static bool setup_platform();
    static bool setup_opengl();
    static bool setup_openal();
#if defined(__EMSCRIPTEN__)
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
    static glm::vec2 screen_size = { 0.f, 0.f };
    static glm::vec2 mouse_position = { 0.f, 0.f };
    static glm::vec2 mouse_position_delta = { 0.f, 0.f };
    static glm::vec2 accumulated_mouse_position_delta = { 0.f, 0.f };
    static glm::float64 time_delta = 0.f; // seconds
    static std::function<void()> start_callback = nullptr;
    static std::function<void()> update_callback = nullptr;
    static std::vector<std::function<void()>> on_audio_locked_callbacks = {};

    static bool is_etc_supported = false;
    static bool is_s3tc_supported = false;
    static bool is_audio_locked = false;
    static bool is_mouse_locked = false;
    static bool is_fullscreen = false;

    static glm::uint gui_mvp_uniform;

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
            if (is_audio_locked) {
                // TODO execute callbacks
                for (const auto& _callback : on_audio_locked_callbacks) {
                    _callback();
                }
                on_audio_locked_callbacks.clear();
            }
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
            detail::keys[_key] = true;
            detail::keys_changed.emplace_back(_key);
            process_lock();
        } else if (event_type == EMSCRIPTEN_EVENT_KEYUP) {
            detail::keys[_key] = false;
            detail::keys_changed.emplace_back(_key);
        }
        return 0;
    }

    EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent* event, void* user_data)
    {
        if (event_type == EMSCRIPTEN_EVENT_MOUSEMOVE) {
            detail::accumulated_mouse_position_delta += glm::vec2((float)event->movementX, (float)event->movementY);
            // std::cout << "x = " << event->screenX << ", y = " << event->screenY << "ok" << std::endl;
            // detail::mouse_position_delta = glm::vec2((float)event->clientX, (float)event->clientY) - detail::mouse_position;
            detail::mouse_position = glm::vec2((float)event->clientX, (float)event->clientY);
            ImGui::GetIO().AddMousePosEvent(mouse_position.x, mouse_position.y);
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) {
            glm::uint _button = event->button;
            detail::buttons[_button] = true;
            detail::buttons_changed.emplace_back(_button);
            ImGui::GetIO().AddMouseButtonEvent(_button, true);
            process_lock();
        } else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
            glm::uint _button = event->button;
            detail::buttons[_button] = false;
            detail::buttons_changed.emplace_back(_button);
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
        glm::vec2 _touch_move_position = glm::vec2((float)_touch_point->clientX, (float)_touch_point->clientY);
        if (event_type == EMSCRIPTEN_EVENT_TOUCHSTART) {
            process_lock();
            detail::mouse_position_delta = { 0, 0 };
        } else if (event_type == EMSCRIPTEN_EVENT_TOUCHMOVE)
            detail::mouse_position_delta = _touch_move_position - detail::mouse_position;
        detail::mouse_position = _touch_move_position;
        return EM_TRUE; // we use preventDefault() for touch callbacks (see Safari on iPad)
    }

#else

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

    static void glfw_mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
    {
        static double _last_x, _last_y;
        double _delta_x = xpos - _last_x;
        double _delta_y = ypos - _last_y;
        _last_x = xpos;
        _last_y = ypos;
        detail::accumulated_mouse_position_delta += glm::vec2((float)_delta_x, (float)_delta_y);
    }

    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
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

    static bool setup_platform()
    {
#if defined(__EMSCRIPTEN__)
        emscripten_assert(emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback));
        emscripten_assert(emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback));
        emscripten_assert(emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback));

        emscripten_assert(emscripten_set_click_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_dblclick_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_mouseenter_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_mouseleave_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_mouseover_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));
        emscripten_assert(emscripten_set_mouseout_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_callback));

        emscripten_assert(emscripten_set_touchstart_callback("#canvas", nullptr, 1, touch_callback));
        // emscripten_assert(emscripten_set_touchend_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touch_callback));
        emscripten_assert(emscripten_set_touchmove_callback("#canvas", nullptr, 1, touch_callback)); // EMSCRIPTEN_EVENT_TARGET_WINDOW doesnt work on safari
        // emscripten_assert(emscripten_set_touchcancel_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, touch_callback));
#else

        glfwSetErrorCallback(glfw_error_callback);

        if (!glfwInit())
            exit(EXIT_FAILURE);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // glfw_window = glfwCreateWindow(1600, 900, "Lucaria", glfwGetPrimaryMonitor(), NULL);
        glfw_window = glfwCreateWindow(1600, 900, "Lucaria", NULL, NULL);
        if (!glfw_window) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        for (const std::pair<int, keyboard_key> _pair : detail::glfw_keyboard_mappings) {
            detail::keys[_pair.second] = false;
        }

        glfwSetKeyCallback(glfw_window, glfw_key_callback);
        glfwSetCursorPosCallback(glfw_window, glfw_mouse_position_callback);
        glfwSetMouseButtonCallback(glfw_window, glfw_mouse_button_callback);

        glfwMakeContextCurrent(glfw_window);
        glfwSetWindowFocusCallback(glfw_window, glfw_window_focus_callback);

        gladLoadGL(glfwGetProcAddress);
        glfwSwapInterval(1);

#endif
        return true;
    }

    static bool setup_opengl()
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
            is_etc_supported = true;
#if LUCARIA_DEBUG
            std::cout << "Extension WEBGL_compressed_texture_etc is supported." << std::endl;
#endif
        }
        if (emscripten_webgl_enable_extension(_webgl_context, "WEBGL_compressed_texture_s3tc")) {
            is_s3tc_supported = true;
#if LUCARIA_DEBUG
            std::cout << "Extension WEBGL_compressed_texture_s3tc is supported." << std::endl;
#endif
        }
#endif

        return true;
    }

    ImGui_ImplOpenGL3_Data* ImGui_ImplOpenGL3_GetBackendData()
    {
        return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL3_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
    }

    ImGuiContext* create_shared_context()
    {
        ImGuiContext* _context = ImGui::CreateContext(imgui_shared_font_atlas.get());
        ImGui::SetCurrentContext(_context);
#if !defined(__EMSCRIPTEN__)
        ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
#endif
        ImGui_ImplOpenGL3_Init("#version 300 es");

        // Tell the GL3 backend we already have a font texture
        if (auto* bd = ImGui_ImplOpenGL3_GetBackendData()) {
            bd->FontTexture = imgui_shared_font_texture;
        }

        return _context;
    }

    void reupload_shared_font_texture_RGBA32()
    {
        unsigned char* pixels = nullptr;
        int w = 0, h = 0;
        imgui_shared_font_atlas->GetTexDataAsRGBA32(&pixels, &w, &h); // builds if needed

        if (imgui_shared_font_texture == 0)
            glGenTextures(1, &imgui_shared_font_texture);

        GLint last_tex = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);
        glBindTexture(GL_TEXTURE_2D, imgui_shared_font_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glBindTexture(GL_TEXTURE_2D, last_tex);

        // Make the atlas point to the shared GL texture id
        imgui_shared_font_atlas->SetTexID((ImTextureID)(intptr_t)imgui_shared_font_texture);
    }

    static void setup_imgui()
    {
        IMGUI_CHECKVERSION();

        imgui_shared_font_atlas = std::make_unique<ImFontAtlas>();
        imgui_shared_font_atlas->AddFontDefault();

        reupload_shared_font_texture_RGBA32();

        imgui_screen_context = create_shared_context();

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

    // static void destroy_openal()
    // {
    //     ALCcontext* _webaudio_context = alcGetCurrentContext();
    //     ALCdevice* _webaudio_device = alcGetContextsDevice(_webaudio_context);
    //     alcMakeContextCurrent(nullptr);
    //     alcDestroyContext(_webaudio_context);
    //     alcCloseDevice(_webaudio_device);
    // }

    void update()
    {

#if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
        static double _last_render_time = 0;
        double _render_time = emscripten_get_now();
        detail::time_delta = (_render_time - _last_render_time) / 1000.f;
#else
        static std::chrono::high_resolution_clock::time_point _last_render_time = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::time_point _render_time = std::chrono::high_resolution_clock::now();
        detail::time_delta = std::chrono::duration<double>(_render_time - _last_render_time).count();
#endif

        int _screen_width, _screen_height;
#if defined(__EMSCRIPTEN__)
        _screen_width = canvas_get_width();
        _screen_height = canvas_get_height();
#else
        glfwGetFramebufferSize(glfw_window, &_screen_width, &_screen_height);
        // glViewport(0, 0, _screen_width, _screen_height);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
        detail::screen_size = { _screen_width, _screen_height };

        if (detail::screen_size == glm::vec2(0.f, 0.f)) {
            return;
        }

        static glm::vec2 _last_accum_pos_delta(0.f, 0.f);
        detail::mouse_position_delta = detail::accumulated_mouse_position_delta - _last_accum_pos_delta;
        _last_accum_pos_delta = detail::accumulated_mouse_position_delta;
        _last_render_time = _render_time;

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(detail::screen_size.x, detail::screen_size.y);

#if !defined(__EMSCRIPTEN__)
        ImGui_ImplGlfw_NewFrame();
#endif

        if (_use_imgui) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
        }
        // io.MouseDown[0] = detail::keys[0];
        // for (auto& _button : detail::buttons_changed)
        //     io.AddMouseButtonEvent(_button, detail::buttons[_button]);
        // io.AddFocusEvent(true);

        ImGui_ImplOpenGL3_Data* _backend_data = static_cast<ImGui_ImplOpenGL3_Data*>(io.BackendRendererUserData);
        gui_mvp_uniform = _backend_data->AttribLocationProjMtx;

        update_mouse_lock();

        update_callback();

        detail::keys_changed.clear();
        detail::buttons_changed.clear();

        // ImGui::ShowDemoWindow();
        if (_use_imgui) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        _use_imgui = _use_imgui_command;

        // wait_fetched_containers();
        // wait_one_fetched_container();
        // std::cout << "drrrr \n";

        // remove_levels();
        // manage();

        graphics_assert();
        if (is_audio_locked) {
            // audio_assert();
        }

#if !defined(__EMSCRIPTEN__)
        glfwSwapBuffers(detail::glfw_window);
        glfwPollEvents();
#endif
    }

    void run_impl(const std::function<void()>& start, const std::function<void()>& update)
    {
        detail::setup_platform();
        detail::setup_opengl();
        detail::setup_imgui();
        detail::update_callback = update;
#if defined(__EMSCRIPTEN__)
        start();
        emscripten_set_main_loop(detail::update, 0, EM_TRUE);
#else
        detail::is_audio_locked = detail::setup_openal();
        start();

        GLint numExtensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

        std::cout << "Supported Extensions (Modern):" << std::endl;
        for (GLint i = 0; i < numExtensions; ++i) {
            const char* extension = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
            std::cout << extension << std::endl;

            if (std::string(extension) == "GL_EXT_texture_compression_s3tc") {
                detail::is_s3tc_supported = true;
            }
        }

        while (!glfwWindowShouldClose(detail::glfw_window)) {
            detail::update();
        }

        destroy_scenes();
        glfwDestroyWindow(detail::glfw_window);
        glfwTerminate();
#endif
    }

    void imgui_special_callback(const ImDrawList* parent_list, const ImDrawCmd* cmd)
    {
        if (cmd->UserCallbackData) {
            glm::mat4& _mvp = *static_cast<glm::mat4*>(cmd->UserCallbackData);
            glUniformMatrix4fv(detail::gui_mvp_uniform, 1, GL_FALSE, &_mvp[0][0]);
            delete &_mvp;
        } else {
            ImDrawData* _draw_data = ImGui::GetDrawData();
            float L = _draw_data->DisplayPos.x;
            float R = _draw_data->DisplayPos.x + _draw_data->DisplaySize.x;
            float T = _draw_data->DisplayPos.y;
            float B = _draw_data->DisplayPos.y + _draw_data->DisplaySize.y;
            const float ortho_projection[4][4] = {
                { 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
                { 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
                { 0.0f, 0.0f, -1.0f, 0.0f },
                { (R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f },
            };
            glUniformMatrix4fv(detail::gui_mvp_uniform, 1, GL_FALSE, &ortho_projection[0][0]);
        }
    }

}

ImDrawList* get_gui_drawlist()
{
    return ImGui::GetBackgroundDrawList();
}

void gui_mvp(const std::optional<glm::mat4>& mvp)
{
    void* _void_mvp = nullptr;
    if (mvp.has_value()) {
        _void_mvp = new glm::mat4(mvp.value());
    }
    ImGui::GetBackgroundDrawList()->AddCallback(detail::imgui_special_callback, _void_mvp);
}

void graphics_assert()
{
#if LUCARIA_DEBUG
    GLenum _gl_err;
    while ((_gl_err = glGetError()) != GL_NO_ERROR) {
        std::string _brief, _description;
        switch (_gl_err) {
        case GL_INVALID_ENUM:
            _brief = "Invalid OpenGL enum";
            _description = "This is given when an enumeration parameter is not a legal enumeration for that function. this is given only for local problems; if the spec allows the enumeration in certain circumstances, where other parameters or state dictate those circumstances, then 'Invalid operation' is the _compilation_result instead";
            break;
        case GL_INVALID_VALUE:
            _brief = "Invalid OpenGL value";
            _description = "This is given when a value parameter is not a legal value for that function. this is only given for local problems; if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances, then 'Invalid operation' is the _compilation_result instead";
            break;
        case GL_INVALID_OPERATION:
            _brief = "Invalid OpenGL operation";
            _description = "This is given when the set of state for a command is not legal for the parameters given to that command. it is also given for commands where combinations of parameters define what the legal parameters are";
            break;
        case GL_OUT_OF_MEMORY:
            _brief = "OpenGL out of memory";
            _description = "This is given when performing an operation that can allocate memory, and the memory cannot be allocated. the _compilation_results of OpenGL functions that return this void are undefined; it is allowable for partial execution of an operation to happen in this circumstance";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            _brief = "Invalid OpenGL framebuffer operation";
            _description = "This is given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete";
            break;
        default:
            _brief = "Unknown OpenGL error";
            _description = "This error hasn't been coded into glite yet, please sumbit an issue report on github.com/127Soft/glite :)";
            break;
        }
        LUCARIA_RUNTIME_ERROR("Failed OpenGL operation with result '" + _brief + "' (" + _description + ")")
    }
#endif
}

void audio_assert()
{
#if LUCARIA_DEBUG
    std::string _reason;
    ALenum _al_error = alGetError();
    if (_al_error != AL_NO_ERROR) {
        if (_al_error == AL_INVALID_NAME)
            _reason = "invalid name";
        else if (_al_error == AL_INVALID_ENUM)
            _reason = " invalid enum";
        else if (_al_error == AL_INVALID_VALUE)
            _reason = " invalid value";
        else if (_al_error == AL_INVALID_OPERATION)
            _reason = " invalid operation";
        else if (_al_error == AL_OUT_OF_MEMORY)
            _reason = "out of memory";
        LUCARIA_RUNTIME_ERROR("Failed OpenAL operation with result '" + _reason + "'")
    }
#endif
}

void on_audio_locked(const std::function<void()>& callback)
{
    if (detail::is_audio_locked) {
        callback();
    } else {
        detail::on_audio_locked_callbacks.emplace_back(callback);
    }
}

std::unordered_map<keyboard_key, bool>& get_keys()
{
    return detail::keys;
}

std::unordered_map<glm::uint, bool>& get_buttons()
{
    return detail::buttons;
}

glm::vec2 get_screen_size()
{
    return detail::screen_size;
}

glm::vec2 get_mouse_position()
{
    return detail::mouse_position;
}

glm::vec2& get_mouse_position_delta()
{
    return detail::mouse_position_delta;
}

glm::float64 get_time_delta()
{
    return detail::time_delta;
    // return 1.0 / 60.0;
}

bool get_is_etc_supported()
{
    return detail::is_etc_supported;
}

bool get_is_s3tc_supported()
{
    return detail::is_s3tc_supported;
}

bool get_is_audio_locked()
{
    return detail::is_audio_locked;
}

bool get_is_mouse_locked()
{
    return detail::is_mouse_locked;
}

}
