#include <fstream>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <core/program.hpp>

extern void graphics_assert();

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
#if LUCARIA_DEBUG
        if (!_result || _log_length > 0) {
            std::vector<GLchar> _result_error_msg(_log_length + 1);
            glGetShaderInfoLog(_shader_id, _log_length, NULL, &_result_error_msg[0]);
            std::cout << "Invalid shader '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
            std::terminate();
        }
#endif
        return _shader_id;
    }

    std::unordered_map<std::string, GLint> enumerate_attributes(const GLuint program_id)
    {
        GLint _attributes_count;
        std::unordered_map<std::string, GLint> _attributes;
        glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTES, &_attributes_count);
        for (GLint _index = 0; _index < _attributes_count; ++_index) {
            char _name[256];
            GLsizei _length;
            GLint _size;
            GLenum _type;
            glGetActiveAttrib(program_id, _index, sizeof(_name), &_length, &_size, &_type, _name);
            _name[_length] = '\0';
            GLint _location = glGetAttribLocation(program_id, _name);
            _attributes[_name] = _location;
#if LUCARIA_DEBUG
            std::cout << "Program has attribute '" << _name << "' at location " << _location << std::endl;
#endif
        }
        return _attributes;
    }

    std::unordered_map<std::string, GLint> enumerate_uniforms(const GLuint program_id)
    {
        GLint _uniform_count;
        glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &_uniform_count);
        std::unordered_map<std::string, GLint> _uniforms;
        for (GLint _index = 0; _index < _uniform_count; ++_index) {
            char _name[256];
            GLsizei _length;
            GLint _size;
            GLenum _type;
            glGetActiveUniform(program_id, _index, sizeof(_name), &_length, &_size, &_type, _name);
            _name[_length] = '\0';
            GLint _location = glGetUniformLocation(program_id, _name);
            _uniforms[_name] = _location;
#if LUCARIA_DEBUG
            std::cout << "Program has uniform '" << _name << "' at location " << _location << std::endl;
#endif
        }
        return _uniforms;

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
#if LUCARIA_DEBUG
    if (_log_length > 0) {
        std::vector<GLchar> _result_error_msg(_log_length + 1);
        glGetProgramInfoLog(_program_id, _log_length, NULL, &_result_error_msg[0]);
        std::cout << "While linking program '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
        if (!_result) {
            std::terminate();
        }
    }
#endif
    glDetachShader(_program_id, _vertex_id);
    glDetachShader(_program_id, _fragment_id);
    glDeleteShader(_vertex_id);
    glDeleteShader(_fragment_id);
    _program_attributes = detail::enumerate_attributes(_program_id);
    _program_uniforms = detail::enumerate_uniforms(_program_id);
    graphics_assert();
}

program_ref::~program_ref()
{
    // glUseProgram(0);
    // glDeleteProgram(_program_id);
}





void program_ref::use() const
{
    glUseProgram(_program_id);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void program_ref::bind(const std::string& name, const mesh_ref& mesh, const mesh_attribute attribute)
{
    _count = mesh.get_count();
    _array_id = mesh.get_array_id();
    std::unordered_map<mesh_attribute, GLuint> _buffer_ids = mesh.get_buffer_ids();
    GLint _location = _program_attributes.at(name);
    GLuint _size = mesh_attribute_sizes.at(attribute);
    glBindVertexArray(_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, _buffer_ids.at(attribute));
    glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(_location);
}

void program_ref::bind(const std::string& name, const cubemap_ref& cubemap, const GLuint slot) const
{
    GLint _location = _program_uniforms.at(name);
    glUniform1i(_location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.get_id());
}

void program_ref::bind(const std::string& name, const texture_ref& texture, const GLuint slot) const
{
    GLint _location = _program_uniforms.at(name);
    glUniform1i(_location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture.get_id());
}

template <> 
void program_ref::bind<GLfloat>(const std::string& name, const GLfloat& value)
{
    const GLint _location = _program_uniforms.at(name);
    glUniform1f(_location, value);
}

template <> 
void program_ref::bind<std::vector<GLfloat>>(const std::string& name, const std::vector<GLfloat>& value)
{
    const GLint _location = _program_uniforms.at(name);
    const GLuint _count = value.size();
    const GLfloat* _ptr = const_cast<const GLfloat*>(value.data());
    glUniform1fv(_location, _count, _ptr);
}

template <> 
void program_ref::bind<glm::vec2>(const std::string& name, const glm::vec2& value)
{
    const GLint _location = _program_uniforms.at(name);
    glUniform2f(_location, value.x, value.y);
}

template <> 
void program_ref::bind<std::vector<glm::vec2>>(const std::string& name, const std::vector<glm::vec2>& value)
{
    const GLint _location = _program_uniforms.at(name);
    const GLuint _count = value.size();
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform2fv(_location, _count, _ptr);
}

template <> 
void program_ref::bind<glm::vec3>(const std::string& name, const glm::vec3& value)
{
    const GLint _location = _program_uniforms.at(name);
    glUniform3f(_location, value.x, value.y, value.z);
}

template <> 
void program_ref::bind<std::vector<glm::vec3>>(const std::string& name, const std::vector<glm::vec3>& value)
{
    const GLint _location = _program_uniforms.at(name);
    const GLuint _count = value.size();
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform3fv(_location, _count, _ptr);
}

template <> 
void program_ref::bind<glm::vec4>(const std::string& name, const glm::vec4& value)
{
    const GLint _location = _program_uniforms.at(name);
    glUniform4f(_location, value.x, value.y, value.z, value.w);
}

template <> 
void program_ref::bind<std::vector<glm::vec4>>(const std::string& name, const std::vector<glm::vec4>& value)
{
    const GLint _location = _program_uniforms.at(name);
    const GLuint _count = value.size();
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform4fv(_location, _count, _ptr);
}

// TODO MATRICES

template <> 
void program_ref::bind<glm::mat4x4>(const std::string& name, const glm::mat4x4& value)
{
    const GLint _location = _program_uniforms.at(name);
    glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(value));
}

void program_ref::draw() const
{
    glBindVertexArray(_array_id);
    glDrawElements(GL_TRIANGLES, _count, GL_UNSIGNED_INT, 0);
}

GLuint program_ref::get_id() const
{
    return _program_id;
}

shader_data load_shader(const std::filesystem::path& file)
{    
#if LUCARIA_DEBUG
    if (!std::filesystem::is_regular_file(file)) {
        std::cout << "Invalid shader path " << file << std::endl;
        std::terminate();
    }
#endif
    shader_data _data;
    std::ifstream _fstream(file, std::ios::binary);
    cereal::PortableBinaryInputArchive _archive(_fstream);
    _archive(_data);
    return _data;
}