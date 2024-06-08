#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

#include <core/program.hpp>

namespace detail {    

    GLuint create_shader(const GLenum type, const std::string& text)
    {
        GLint _log_length;
        GLint _result = GL_FALSE;
        GLuint _shader_id = glCreateShader(type);
        const GLchar* _source_ptr = text.c_str();
        glShaderSource(_shader_id, 1, &_source_ptr, NULL);
        glCompileShader(_shader_id);
        glGetShaderiv(_shader_id, GL_COMPILE_STATUS, &_result);
        glGetShaderiv(_shader_id, GL_INFO_LOG_LENGTH, &_log_length);
#if DEBUG
        if (!_result && _log_length > 0) {
            std::vector<GLchar> _result_error_msg(_log_length + 1);
            glGetShaderInfoLog(_shader_id, _log_length, NULL, &_result_error_msg[0]);
            std::cerr << "Invalid shader '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
            std::terminate();
        }
#endif
        return _shader_id;
    }

}

program_ref::program_ref(const shader_data& vertex, const shader_data& fragment)
{
    GLuint _vertex_id = detail::create_shader(GL_VERTEX_SHADER, vertex.text);
    GLuint _fragment_id = detail::create_shader(GL_FRAGMENT_SHADER, fragment.text);
    GLint _log_length;
    GLint _result = GL_FALSE;
    _program_id = glCreateProgram();
    glAttachShader(_program_id, _vertex_id);
    glAttachShader(_program_id, _fragment_id);
    glLinkProgram(_program_id);
    glGetProgramiv(_program_id, GL_LINK_STATUS, &_result);
    glGetProgramiv(_program_id, GL_INFO_LOG_LENGTH, &_log_length);
#if DEBUG
    if (!_result && _log_length > 0) {
        std::vector<char> _result_error_msg(_log_length + 1);
        glGetProgramInfoLog(_program_id, _log_length, NULL, &_result_error_msg[0]);
        std::cerr << "Invalid program '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
        std::terminate();
    }
#endif
    glDetachShader(_program_id, _vertex_id);
    glDetachShader(_program_id, _fragment_id);
    glDeleteShader(_vertex_id);
    glDeleteShader(_fragment_id);
}

program_ref::~program_ref()
{
    glUseProgram(0);
    glDeleteProgram(_program_id);
}

GLuint program_ref::get_id() const
{
    return _program_id;
}

shader_data load_shader(const std::filesystem::path& file)
{
    shader_data _data;
    std::ofstream _fstream(file);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
    _archive(_data.text);
    return _data;
}