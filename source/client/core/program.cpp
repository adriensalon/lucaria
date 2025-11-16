
#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/error.hpp>
#include <lucaria/core/opengl.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {
namespace {

    [[nodiscard]] static glm::uint create_shader(const GLenum type, const std::string& text)
    {
        glm::int32 _log_length;
        glm::int32 _result = GL_FALSE;
        glm::uint _shader_id = glCreateShader(type);
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
            if (!_result) {
                LUCARIA_RUNTIME_ERROR("Failed compiling shader")
            }
        }
#endif
        return _shader_id;
    }

    [[nodiscard]] static std::unordered_map<std::string, glm::int32> enumerate_attributes(const glm::uint program_id)
    {
        glm::int32 _attributes_count;
        std::unordered_map<std::string, glm::int32> _attributes;
        glGetProgramiv(program_id, GL_ACTIVE_ATTRIBUTES, &_attributes_count);
        for (glm::int32 _index = 0; _index < _attributes_count; ++_index) {
            char _name[256];
            GLsizei _length;
            glm::int32 _size;
            GLenum _type;
            glGetActiveAttrib(program_id, _index, sizeof(_name), &_length, &_size, &_type, _name);
            _name[_length] = '\0';
            glm::int32 _location = glGetAttribLocation(program_id, _name);
            _attributes[_name] = _location;
#if LUCARIA_DEBUG
            std::cout << "Program has attribute '" << _name << "' at location " << _location << std::endl;
#endif
        }
        return _attributes;
    }

    [[nodiscard]] static std::unordered_map<std::string, glm::int32> enumerate_uniforms(const glm::uint program_id)
    {
        glm::int32 _uniform_count;
        glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &_uniform_count);
        std::unordered_map<std::string, glm::int32> _uniforms;
        for (glm::int32 _index = 0; _index < _uniform_count; ++_index) {
            char _name[256];
            GLsizei _length;
            glm::int32 _size;
            GLenum _type;
            glGetActiveUniform(program_id, _index, sizeof(_name), &_length, &_size, &_type, _name);
            _name[_length] = '\0';
            glm::int32 _location = glGetUniformLocation(program_id, _name);
            _uniforms[_name] = _location;
#if LUCARIA_DEBUG
            std::cout << "Program has uniform '" << _name << "' at location " << _location << std::endl;
#endif
        }
        return _uniforms;
    }

}

program::program(program&& other)
{
    *this = std::move(other);
}

program& program::operator=(program&& other)
{
    _is_owning = true;
    _handle = other._handle;
    _attributes = std::move(other._attributes);
    _uniforms = std::move(other._uniforms);
    _bound_array_id = other._bound_array_id;
    _bound_indices_count = other._bound_indices_count;
    other._is_owning = false;
    return *this;
}

program::~program()
{
    if (_is_owning) {
        glUseProgram(0);
        glDeleteProgram(_handle);
    }
}

program::program(const shader& vertex, const shader& fragment)
{
    glm::uint _vertex_id = create_shader(GL_VERTEX_SHADER, vertex.data.text);
    glm::uint _fragment_id = create_shader(GL_FRAGMENT_SHADER, fragment.data.text);
    glm::int32 _log_length;
    glm::int32 _result = GL_FALSE;
    _handle = glCreateProgram();
    glAttachShader(_handle, _vertex_id);
    glAttachShader(_handle, _fragment_id);
    glLinkProgram(_handle);
    glGetProgramiv(_handle, GL_LINK_STATUS, &_result);
    glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &_log_length);
#if LUCARIA_DEBUG
    if (_log_length > 0) {
        std::vector<GLchar> _result_error_msg(_log_length + 1);
        glGetProgramInfoLog(_handle, _log_length, NULL, &_result_error_msg[0]);
        std::cout << "While linking program '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
        if (!_result) {
            LUCARIA_RUNTIME_ERROR("Failed linking program")
        }
    }
#endif
    glDetachShader(_handle, _vertex_id);
    glDetachShader(_handle, _fragment_id);
    glDeleteShader(_vertex_id);
    glDeleteShader(_fragment_id);
    _attributes = enumerate_attributes(_handle);
    _uniforms = enumerate_uniforms(_handle);
}

void program::use() const
{
    glUseProgram(_handle);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void program::bind_attribute(const std::string& name, const mesh& from, const mesh_attribute attribute)
{
    _bound_indices_count = from.get_size();
    _bound_array_id = from.get_array_handle();
    const std::unordered_map<mesh_attribute, glm::uint>& _attribute_handles = from.get_attribute_handles();
    if (_attributes.find(name) == _attributes.end()) {
        LUCARIA_RUNTIME_ERROR("Name " + name + " not found in shader")
    }
    glm::int32 _location = _attributes.at(name);
    glm::uint _size = detail::mesh_attribute_sizes.at(attribute);
    glBindVertexArray(_bound_array_id);
    if (_attribute_handles.find(attribute) == _attribute_handles.end()) {
        LUCARIA_RUNTIME_ERROR("Attribute " + std::to_string(static_cast<int>(attribute)) + " is not in mesh")
    }
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_handles.at(attribute));
    if (attribute == mesh_attribute::bones) {
        glVertexAttribIPointer(_location, _size, GL_INT, _size * sizeof(glm::int32), (void*)0);
    } else {
        glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(glm::float32), (void*)0);
    }
    glEnableVertexAttribArray(_location);
}

void program::bind_attribute(const std::string& name, const viewport& from, const mesh_attribute attribute)
{
    if (attribute != mesh_attribute::position && attribute != mesh_attribute::texcoord) {
        LUCARIA_RUNTIME_ERROR("Failed to bind viewport attribute because only position or texcoord is expected")
    }
    _bound_indices_count = from.get_size();
    _bound_array_id = from.get_array_handle();
    const std::unordered_map<mesh_attribute, glm::uint> _attribute_handles = {
        { mesh_attribute::position, from.get_positions_handle() },
        { mesh_attribute::texcoord, from.get_texcoords_handle() }
    };
    if (_attributes.find(name) == _attributes.end()) {
        LUCARIA_RUNTIME_ERROR("Name " + name + " not found in shader")
    }
    glm::int32 _location = _attributes.at(name);
    glm::uint _size = detail::mesh_attribute_sizes.at(attribute);
    glBindVertexArray(_bound_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, _attribute_handles.at(attribute));
    if (attribute == mesh_attribute::bones) {
        glVertexAttribIPointer(_location, _size, GL_INT, _size * sizeof(glm::int32), (void*)0);
    } else {
        glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(glm::float32), (void*)0);
    }
    glEnableVertexAttribArray(_location);
}

void program::bind_uniform(const std::string& name, const cubemap& from, const glm::uint slot) const
{
    glm::int32 _location = _uniforms.at(name);
    glUniform1i(_location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, from.get_handle());
}

void program::bind_uniform(const std::string& name, const texture& from, const glm::uint slot) const
{
    glm::int32 _location = _uniforms.at(name);
    glUniform1i(_location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, from.get_handle());
}

template <>
void program::bind_uniform<glm::int32>(const std::string& name, const glm::int32& value)
{
    const glm::int32 _location = _uniforms.at(name);
    glUniform1i(_location, value);
}

template <>
void program::bind_uniform<GLfloat>(const std::string& name, const GLfloat& value)
{
    const glm::int32 _location = _uniforms.at(name);
    glUniform1f(_location, value);
}

template <>
void program::bind_uniform<std::vector<GLfloat>>(const std::string& name, const std::vector<GLfloat>& value)
{
    const glm::int32 _location = _uniforms.at(name);
    const glm::uint _count = static_cast<glm::uint>(value.size());
    const GLfloat* _ptr = const_cast<const GLfloat*>(value.data());
    glUniform1fv(_location, _count, _ptr);
}

template <>
void program::bind_uniform<glm::vec2>(const std::string& name, const glm::vec2& value)
{
    const glm::int32 _location = _uniforms.at(name);
    glUniform2f(_location, value.x, value.y);
}

template <>
void program::bind_uniform<std::vector<glm::vec2>>(const std::string& name, const std::vector<glm::vec2>& value)
{
    const glm::int32 _location = _uniforms.at(name);
    const glm::uint _count = static_cast<glm::uint>(value.size());
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform2fv(_location, _count, _ptr);
}

template <>
void program::bind_uniform<glm::vec3>(const std::string& name, const glm::vec3& value)
{
    const glm::int32 _location = _uniforms.at(name);
    glUniform3f(_location, value.x, value.y, value.z);
}

template <>
void program::bind_uniform<std::vector<glm::vec3>>(const std::string& name, const std::vector<glm::vec3>& value)
{
    const glm::int32 _location = _uniforms.at(name);
    const glm::uint _count = static_cast<glm::uint>(value.size());
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform3fv(_location, _count, _ptr);
}

template <>
void program::bind_uniform<glm::vec4>(const std::string& name, const glm::vec4& value)
{
    const glm::int32 _location = _uniforms.at(name);
    glUniform4f(_location, value.x, value.y, value.z, value.w);
}

template <>
void program::bind_uniform<std::vector<glm::vec4>>(const std::string& name, const std::vector<glm::vec4>& value)
{
    const glm::int32 _location = _uniforms.at(name);
    const glm::uint _count = static_cast<glm::uint>(value.size());
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform4fv(_location, _count, _ptr);
}

// TODO MATRICES

template <>
void program::bind_uniform<glm::mat4x4>(const std::string& name, const glm::mat4x4& value)
{
    const glm::int32 _location = _uniforms.at(name);
    glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(value));
}

template <>
void program::bind_uniform<std::vector<glm::mat4x4>>(const std::string& name, const std::vector<glm::mat4x4>& value)
{
    const glm::int32 _location = _uniforms.at(name);
    const glm::uint _count = static_cast<glm::uint>(value.size());
    const glm::float32* _ptr = reinterpret_cast<const glm::float32*>(value.data());
    glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
}

template <>
void program::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
{
    const glm::int32 _location = _uniforms.at(name);
    const glm::uint _count = static_cast<glm::uint>(value.size());
    const glm::float32* _ptr = reinterpret_cast<const glm::float32*>(value.data());
    glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
}

void program::draw(const bool use_depth) const
{
    if (use_depth) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
    } else {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }
    glBindVertexArray(_bound_array_id);
    glDrawElements(GL_TRIANGLES, _bound_indices_count, GL_UNSIGNED_INT, 0);
}

#if LUCARIA_GUIZMO
void program::bind_guizmo(const std::string& name, const detail::guizmo_mesh& from)
{
    _bound_indices_count = from.get_size();
    _bound_array_id = from.get_array_handle();
    glm::uint _positions_id = from.get_positions_handle();
    glm::int32 _location = _attributes.at(name);
    glm::uint _size = 3;
    glBindVertexArray(_bound_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, _positions_id);
    glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(_location);
}

void program::draw_guizmo() const
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glBindVertexArray(_bound_array_id);
    glDrawElements(GL_LINES, _bound_indices_count, GL_UNSIGNED_INT, 0);
}
#endif

glm::uint program::get_handle() const
{
    return _handle;
}

const std::unordered_map<std::string, glm::int32>& program::get_attributes() const
{
    return _attributes;
}

const std::unordered_map<std::string, glm::int32>& program::get_uniforms() const
{
    return _uniforms;
}

fetched<program> fetch_program(const std::filesystem::path& vertex_data_path, const std::filesystem::path& fragment_data_path)
{
    std::vector<std::filesystem::path> _shaders_paths = { vertex_data_path, fragment_data_path };
    std::shared_ptr<std::promise<std::pair<shader, shader>>> _shaders_promise = std::make_shared<std::promise<std::pair<shader, shader>>>();

    detail::fetch_bytes(_shaders_paths, [_shaders_promise](const std::vector<std::vector<char>>& _data_bytes) {
        std::pair<shader, shader> _shaders = {
            shader(_data_bytes[0]),
            shader(_data_bytes[1])
        };
        _shaders_promise->set_value(std::move(_shaders));
    });

    return fetched<program>(_shaders_promise->get_future(), [](const std::pair<shader, shader>& _from) {
        return program(_from.first, _from.second);
    });
}

}
