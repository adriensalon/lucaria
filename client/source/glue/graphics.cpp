
#include <iostream>
#include <optional>

#include <GLES3/gl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <tiny_gltf.h>

extern glm::vec2 get_screen_size();
extern void emscripten_assert(EMSCRIPTEN_RESULT result);

namespace detail {

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
        std::cerr << "WebGL2 context cannot be created on this device" << std::endl;
        std::terminate();        
    }
    emscripten_assert(emscripten_webgl_make_context_current(_webgl_context));
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = NULL;
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    return true;
}

static bool is_graphics_setup = setup_opengl();

}

void graphics_assert()
{
#if DEBUG
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
        std::cerr << "Invalid OpenGL _compilation_result '" << _brief << "' (" << _description << ")" << std::endl;
        std::terminate();
    }
#endif
}
