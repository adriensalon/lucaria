
#include <iostream>

#include <GLES3/gl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <imgui.h>

#include <glue/window.hpp>

namespace detail {

struct ImGui_ImplOpenGL3_Data {
    GLuint GlVersion; // Extracted at runtime using GL_MAJOR_VERSION, GL_MINOR_VERSION queries (e.g. 320 for GL 3.2)
    char GlslVersionString[32]; // Specified by user or detected based on compile time GL settings.
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

extern void create_openal_context_if_needed();
extern bool is_openal_context_created();
extern void destroy_openal_context();

static bool setup_emscripten();
static bool static_setup = setup_emscripten();

static std::unordered_map<std::string, bool> keys = {};
static std::vector<std::string> keys_changed = {};
static std::unordered_map<int, bool> buttons = {};
static std::vector<int> buttons_changed = {};
static glm::vec2 screen_size = { 0.f, 0.f };
static glm::vec2 mouse_position = { 0.f, 0.f };
static glm::vec2 mouse_position_delta = { 0.f, 0.f };
static float time_delta = 0.f;
static std::function<void()> update_callback = nullptr;

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

EM_BOOL key_callback(int event_type, const EmscriptenKeyboardEvent* event, void* user_data)
{
    // https://github.com/emscripten-core/emscripten/blob/main/test/test_html5_core.c
    if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) {
        create_openal_context_if_needed();
        std::string _key_down(event->key);
        detail::keys[_key_down] = true;
        detail::keys_changed.emplace_back(_key_down);
    } else if (event_type == EMSCRIPTEN_EVENT_KEYUP) {
        std::string _key_up(event->key);
        detail::keys[_key_up] = false;
        detail::keys_changed.emplace_back(_key_up);
    }
    return 0;
}

EM_BOOL mouse_callback(int event_type, const EmscriptenMouseEvent* event, void* user_data)
{
    // https://github.com/emscripten-core/emscripten/blob/main/test/test_html5_core.c
    if (event_type == EMSCRIPTEN_EVENT_MOUSEMOVE) {
        detail::mouse_position_delta = glm::vec2((float)event->movementX, (float)event->movementY);
        detail::mouse_position = glm::vec2((float)event->clientX, (float)event->clientY);
        ImGui::GetIO().AddMousePosEvent(mouse_position.x, mouse_position.y);
    }
    if (event_type == EMSCRIPTEN_EVENT_MOUSEDOWN) {
        create_openal_context_if_needed();
        unsigned int _button = event->button;
        detail::buttons[_button] = true;
        detail::buttons_changed.emplace_back(_button);
    } else if (event_type == EMSCRIPTEN_EVENT_MOUSEUP) {
        unsigned int _button = event->button;
        detail::buttons[_button] = false;
        detail::buttons_changed.emplace_back(_button);
    }
    if (event_type == EMSCRIPTEN_EVENT_CLICK) // we lock / unlock the pointer on click
    {
        create_openal_context_if_needed();
        EmscriptenPointerlockChangeEvent _pointer_lock;
        emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
        if (!_pointer_lock.isActive)
            emscripten_assert(emscripten_request_pointerlock("#canvas", 1));
        // else
        // {
        // 	emscripten_assert(emscripten_exit_pointerlock());
        // 	emscripten_assert(emscripten_get_pointerlock_status(&_pointer_lock));
        // 	if (_pointer_lock.isActive)
        // 		throw "Error : pointer lock exit did not work";
        // }
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
        create_openal_context_if_needed();
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

void update()
{
    static double _last_render_time = 0;
    double _render_time = emscripten_get_now();
    double _time_delta = _render_time - _last_render_time;
    _last_render_time = _render_time;

    int _screen_width = canvas_get_width();
    int _screen_height = canvas_get_height();
    detail::screen_size = { _screen_width, _screen_height };
    detail::time_delta = _time_delta;

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

    update_callback();

    detail::mouse_position_delta = { 0, 0 };
    detail::keys_changed.clear();
    detail::buttons_changed.clear();

    // ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}

void run(std::function<void()> update)
{
    detail::update_callback = update;
    emscripten_set_main_loop(detail::update, 0, EM_TRUE);
}

std::unordered_map<std::string, bool>& get_keys()
{
    return detail::keys;
}

std::unordered_map<int, bool>& get_buttons()
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

glm::vec2 get_mouse_position_delta()
{
    return detail::mouse_position_delta;
}

float get_time_delta()
{
    return detail::time_delta;
}

std::size_t get_samplerate()
{
    return static_cast<std::size_t>(detail::browser_get_samplerate());
}
