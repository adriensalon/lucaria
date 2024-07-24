
#include <iostream>

#include <AL/al.h>
#include <AL/alc.h>
#include <GLES3/gl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <imgui.h>

#include <core/fetch.hpp>
#include <core/window.hpp>

namespace detail {

struct ImGui_ImplOpenGL3_Data {
    glm::uint GlVersion; // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
    char GlslVersionString[32]; // Specified by user or detected based on compile time GL settings.
    glm::uint FontTexture;
    glm::uint ShaderHandle;
    GLint AttribLocationTex; // Uniforms location
    GLint AttribLocationProjMtx;
    glm::uint AttribLocationVtxPos; // Vertex attributes location
    glm::uint AttribLocationVtxUV;
    glm::uint AttribLocationVtxColor;
    glm::uint VboHandle, ElementsHandle;
    GLsizeiptr VertexBufferSize;
    GLsizeiptr IndexBufferSize;
    bool HasClipOrigin;
    bool UseBufferSubData;

    ImGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
};

static bool setup_emscripten();
static bool setup_opengl();
static bool setup_openal();
static bool static_setup = setup_emscripten() && setup_opengl();

static std::unordered_map<std::string, bool> keys = {};
static std::vector<std::string> keys_changed = {};
static std::unordered_map<glm::uint, bool> buttons = {};
static std::vector<glm::uint> buttons_changed = {};
static glm::vec2 screen_size = { 0.f, 0.f };
static glm::vec2 mouse_position = { 0.f, 0.f };
static glm::vec2 mouse_position_delta = { 0.f, 0.f };
static glm::vec2 accumulated_mouse_position_delta = { 0.f, 0.f };
static glm::float64 time_delta = 0.f; // seconds
static std::function<void()> update_callback = nullptr;
static std::vector<std::function<void()>> on_audio_locked_callbacks = {};

static bool is_etc_supported = false;
static bool is_s3tc_supported = false;
static bool is_audio_locked = false;
static bool is_mouse_locked = false;

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
            std::cout << "Invalid emscripten result '" << _brief << "'" << std::endl;
            std::terminate();
        }
    }
#endif
}

void update_mouse_lock()
{
    EmscriptenPointerlockChangeEvent _pointer_lock;
    emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
    is_mouse_locked = _pointer_lock.isActive;
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
    if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
        std::string _key_down(event->key);
        detail::keys[_key_down] = true;
        detail::keys_changed.emplace_back(_key_down);
        process_lock();
    } else if (event_type == EMSCRIPTEN_EVENT_KEYUP) {
        std::string _key_up(event->key);
        detail::keys[_key_up] = false;
        detail::keys_changed.emplace_back(_key_up);
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

static bool setup_emscripten()
{
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
    return true;
}

static bool setup_opengl()
{
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
        std::cout << "WebGL2 context cannot be created on this device" << std::endl;
        std::terminate();
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = NULL;
    ImGui::StyleColorsLight();
    ImGui_ImplOpenGL3_Init("#version 300 es");

    return true;
}

static bool setup_openal()
{
    // https://emscripten.org/docs/porting/Audio.html
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

void update()
{
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
    static double _last_render_time = 0;
    double _render_time = emscripten_get_now();
    detail::time_delta = (_render_time - _last_render_time) / 1000.f;
    _last_render_time = _render_time;

    static glm::vec2 _last_accum_pos_delta(0.f, 0.f);
    detail::mouse_position_delta = detail::accumulated_mouse_position_delta - _last_accum_pos_delta;
    _last_accum_pos_delta = detail::accumulated_mouse_position_delta;

    int _screen_width = canvas_get_width();
    int _screen_height = canvas_get_height();
    detail::screen_size = { _screen_width, _screen_height };

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(detail::screen_size.x, detail::screen_size.y);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    // io.
    io.MouseDown[0] = detail::keys[0];
    // for (auto& _button : detail::buttons_changed)
    //     io.AddMouseButtonEvent(_button, detail::buttons[_button]);
    // io.AddFocusEvent(true);

    ImGui_ImplOpenGL3_Data* _backend_data = static_cast<ImGui_ImplOpenGL3_Data*>(io.BackendRendererUserData);
    const auto _projection_matrix_index = _backend_data->AttribLocationProjMtx;
    // std::cout << _projection_matrix_index << std::endl; location of proj mat

    update_mouse_lock();

    update_callback();

    // detail::mouse_position_delta = { 0, 0 };
    detail::keys_changed.clear();
    detail::buttons_changed.clear();

    // ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    wait_fetched_containers();
    // detail::mouse_position_delta = glm::vec2(0.f);

    graphics_assert();
    if (is_audio_locked) {
        audio_assert();
    }
}

}

void run(std::function<void()> update)
{
    detail::update_callback = update;
    emscripten_set_main_loop(detail::update, 0, EM_TRUE);
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
        std::cout << "Invalid OpenGL result '" << _brief << "' (" << _description << ")" << std::endl;
        std::terminate();
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
        std::cout << "Invalid OpenAL result '" << _reason << "'" << std::endl;
        std::terminate();
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

std::unordered_map<std::string, bool>& get_keys()
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
