#include <exception>
#include <iostream>
#include <string>

#include <AL/al.h>
#include <AL/alc.h>

#include <lucaria/core/error.hpp>

#if LUCARIA_PLATFORM_ANDROID
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#elif LUCARIA_PLATFORM_WEB
#include <GLES3/gl3.h>
#elif LUCARIA_PLATFORM_WIN32
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

namespace lucaria {

void runtime_error(std::string_view file, const int line, const std::string& message)
{
    std::string _text;
    _text.append("Lucaria error in '").append(file.data()).append("' ");
    _text.append("line '").append(std::to_string(line)).append(" ");
    _text.append("with message: ").append(message);
    std::cout << _text << std::endl;
#if defined(__EMSCRIPTEN__)
    std::terminate();
#else
    throw std::runtime_error(_text);
#endif
}

void runtime_openal_assert(std::string_view file, const int line)
{
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
        runtime_error(file, line, "Failed OpenAL operation with result '" + _reason + "'");
    }
}

void runtime_opengl_assert(std::string_view file, const int line)
{
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
        runtime_error(file, line, "Failed OpenGL operation with result '" + _brief + "' (" + _description + ")");
    }
}

}