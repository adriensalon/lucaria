#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/graphics.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/hash.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/load.hpp>

namespace detail {

glm::uint create_shader(const GLenum type, const std::string& text)
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
            std::terminate();
        }
    }
#endif
    return _shader_id;
}

std::unordered_map<std::string, glm::int32> enumerate_attributes(const glm::uint program_id)
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

std::unordered_map<std::string, glm::int32> enumerate_uniforms(const glm::uint program_id)
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

static std::unordered_map<std::size_t, std::pair<std::vector<shader_data>, std::promise<std::shared_ptr<program_ref>>>> promises;

}

program_ref::program_ref(program_ref&& other)
{
    *this = std::move(other);
}

program_ref& program_ref::operator=(program_ref&& other)
{
    _program_id = other._program_id;
    _array_id = other._array_id;
    _indices_count = other._indices_count;
    _program_attributes = std::move(other._program_attributes);
    _program_uniforms = std::move(other._program_uniforms);
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

program_ref::~program_ref()
{
    if (_is_instanced) {
        glUseProgram(0);
        glDeleteProgram(_program_id);
    }
}

program_ref::program_ref(const shader_data& vertex, const shader_data& fragment)
{
    glm::uint _vertex_id = detail::create_shader(GL_VERTEX_SHADER, vertex.text);
    glm::uint _fragment_id = detail::create_shader(GL_FRAGMENT_SHADER, fragment.text);
    glm::int32 _log_length;
    glm::int32 _result = GL_FALSE;
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
    _indices_count = mesh.get_indices_count();
    _array_id = mesh.get_array_id();
    std::unordered_map<mesh_attribute, glm::uint> _buffer_ids = mesh.get_buffer_ids();
#if LUCARIA_DEBUG
    if (_program_attributes.find(name) == _program_attributes.end()) {
        std::cout << "Name " << name << " not found in shader." << std::endl;
        std::terminate();
    }
#endif
    glm::int32 _location = _program_attributes.at(name);
    glm::uint _size = mesh_attribute_sizes.at(attribute);
    glBindVertexArray(_array_id);
#if LUCARIA_DEBUG
    if (_buffer_ids.find(attribute) == _buffer_ids.end()) {
        std::cout << "Attribute " << (int)attribute << " is not in mesh." << std::endl;
        std::terminate();
    }
#endif
    glBindBuffer(GL_ARRAY_BUFFER, _buffer_ids.at(attribute));
    if (attribute == mesh_attribute::bones) {
        glVertexAttribIPointer(_location, _size, GL_INT, _size * sizeof(glm::int32), (void*)0);
    } else {
        glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(glm::float32), (void*)0);
    }
    glEnableVertexAttribArray(_location);
}

#if LUCARIA_GUIZMO

void program_ref::bind_guizmo(const std::string& name, const guizmo_mesh_ref& mesh)
{
    _indices_count = mesh.get_indices_count();
    _array_id = mesh.get_array_id();
    glm::uint _positions_id = mesh.get_positions_id();
    glm::int32 _location = _program_attributes.at(name);
    glm::uint _size = 3;
    glBindVertexArray(_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, _positions_id);
    glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(_location);
}

#endif

void program_ref::bind(const std::string& name, const cubemap_ref& cubemap, const glm::uint slot) const
{
    glm::int32 _location = _program_uniforms.at(name);
    glUniform1i(_location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.get_id());
}

void program_ref::bind(const std::string& name, const texture_ref& texture, const glm::uint slot) const
{
    glm::int32 _location = _program_uniforms.at(name);
    glUniform1i(_location, slot);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture.get_id());
}

template <>
void program_ref::bind<glm::int32>(const std::string& name, const glm::int32& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    glUniform1i(_location, value);
}

template <>
void program_ref::bind<GLfloat>(const std::string& name, const GLfloat& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    glUniform1f(_location, value);
}

template <>
void program_ref::bind<std::vector<GLfloat>>(const std::string& name, const std::vector<GLfloat>& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    const glm::uint _count = value.size();
    const GLfloat* _ptr = const_cast<const GLfloat*>(value.data());
    glUniform1fv(_location, _count, _ptr);
}

template <>
void program_ref::bind<glm::vec2>(const std::string& name, const glm::vec2& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    glUniform2f(_location, value.x, value.y);
}

template <>
void program_ref::bind<std::vector<glm::vec2>>(const std::string& name, const std::vector<glm::vec2>& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    const glm::uint _count = value.size();
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform2fv(_location, _count, _ptr);
}

template <>
void program_ref::bind<glm::vec3>(const std::string& name, const glm::vec3& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    glUniform3f(_location, value.x, value.y, value.z);
}

template <>
void program_ref::bind<std::vector<glm::vec3>>(const std::string& name, const std::vector<glm::vec3>& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    const glm::uint _count = value.size();
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform3fv(_location, _count, _ptr);
}

template <>
void program_ref::bind<glm::vec4>(const std::string& name, const glm::vec4& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    glUniform4f(_location, value.x, value.y, value.z, value.w);
}

template <>
void program_ref::bind<std::vector<glm::vec4>>(const std::string& name, const std::vector<glm::vec4>& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    const glm::uint _count = value.size();
    const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
    glUniform4fv(_location, _count, _ptr);
}

// TODO MATRICES

template <>
void program_ref::bind<glm::mat4x4>(const std::string& name, const glm::mat4x4& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(value));
}

template <>
void program_ref::bind<std::vector<glm::mat4x4>>(const std::string& name, const std::vector<glm::mat4x4>& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    const glm::uint _count = value.size();
    const glm::float32* _ptr = reinterpret_cast<const glm::float32*>(value.data());
    glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
}

template <>
void program_ref::bind<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
{
    const glm::int32 _location = _program_uniforms.at(name);
    const glm::uint _count = value.size();
    const glm::float32* _ptr = reinterpret_cast<const glm::float32*>(value.data());
    glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
    
}

// static bool _depth_test_enabled = true;

void program_ref::draw(const bool use_depth) const
{
    // if (_depth_test_enabled != use_depth) {
        if (use_depth) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
        }
        // _depth_test_enabled = use_depth;
    // }
    glBindVertexArray(_array_id);
    glDrawElements(GL_TRIANGLES, _indices_count, GL_UNSIGNED_INT, 0);
}

#if LUCARIA_GUIZMO

void program_ref::draw_guizmo() const
{
    // if (_depth_test_enabled) {
            glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        // _depth_test_enabled = true;
    // }
    glBindVertexArray(_array_id);
    glDrawElements(GL_LINES, _indices_count, GL_UNSIGNED_INT, 0);
}

#endif

glm::uint program_ref::get_id() const
{
    return _program_id;
}

shader_data load_shader_data(const std::vector<char>& shader_bytes)
{
    shader_data _data;
    {
        raw_input_stream _stream(shader_bytes);
#if LUCARIA_JSON
        cereal::JSONInputArchive _archive(_stream);
#else
        cereal::PortableBinaryInputArchive _archive(_stream);
#endif
        _archive(_data);
    }    
    return _data;
}

std::future<std::shared_ptr<program_ref>> fetch_program(const std::filesystem::path& vertex_shader_path, const std::filesystem::path& fragment_shader_path)
{
    const std::size_t _hash = path_vector_hash()({ vertex_shader_path, fragment_shader_path });
    std::pair<std::vector<shader_data>, std::promise<std::shared_ptr<program_ref>>>& _promise_pair = detail::promises[_hash];
    fetch_files({ vertex_shader_path.string(), fragment_shader_path.string() }, [&_promise_pair](const std::size_t, const std::size_t, const std::vector<char>& shader_bytes) {
        _promise_pair.first.emplace_back(load_shader_data(shader_bytes));
        if (_promise_pair.first.size() == 2) {
            _promise_pair.second.set_value(std::make_shared<program_ref>(_promise_pair.first[0], _promise_pair.first[1]));
        }
    });
    return _promise_pair.second.get_future();
}

void clear_program_fetches()
{
    detail::promises.clear();
}