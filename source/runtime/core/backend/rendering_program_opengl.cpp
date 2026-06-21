#include <cstdint>

#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/rendering_program.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static const std::unordered_map<data_vertex_attribute, uint32> _mesh_attribute_sizes = {
            { data_vertex_attribute::position, 3 },
            { data_vertex_attribute::color, 3 },
            { data_vertex_attribute::normal, 3 },
            { data_vertex_attribute::tangent, 3 },
            { data_vertex_attribute::bitangent, 3 },
            { data_vertex_attribute::texcoord, 2 },
            { data_vertex_attribute::bones, 4 },
            { data_vertex_attribute::weights, 4 },
        };

        [[nodiscard]] static GLuint _make_shader(const GLenum type, const std::string& text)
        {
            GLint _log_length;
            GLint _result = GL_FALSE;
            GLuint _shader_id = glCreateShader(type);
            const GLchar* _source_ptr = text.c_str();
            glShaderSource(_shader_id, 1, &_source_ptr, NULL);
            glCompileShader(_shader_id);
            glGetShaderiv(_shader_id, GL_COMPILE_STATUS, &_result);
            glGetShaderiv(_shader_id, GL_INFO_LOG_LENGTH, &_log_length);
#if defined(LUCARIA_DEBUG)
            if (!_result || _log_length > 0) {
                std::vector<GLchar> _result_error_msg(_log_length + 1);
                glGetShaderInfoLog(_shader_id, _log_length, NULL, &_result_error_msg[0]);
                std::cout << "Invalid object_shader '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
                LUCARIA_DEBUG_ASSERT(_result, "Failed compiling object_shader")
            }
#endif
            return _shader_id;
        }

        [[nodiscard]] static std::unordered_map<std::string, GLint> _reflect_attributes(const GLuint program_id)
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
// #if defined(LUCARIA_DEBUG)
//                 std::cout << "Program has attribute '" << _name << "' at location " << _location << std::endl;
// #endif
            }
            return _attributes;
        }

        [[nodiscard]] static std::unordered_map<std::string, GLint> _reflect_uniforms(const GLuint program_id)
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
// #if defined(LUCARIA_DEBUG)
//                 std::cout << "Program has uniform '" << _name << "' at location " << _location << std::endl;
// #endif
            }
            return _uniforms;
        }

    }

    rendering_program::~rendering_program()
    {
        if (ownership.owns()) {
            glUseProgram(0);
            glDeleteProgram(id);
        }
    }

    rendering_program::rendering_program(const object_shader& vertex, const object_shader& fragment)
    {
        GLuint _vertex_id = _make_shader(GL_VERTEX_SHADER, vertex.data.text);
        GLuint _fragment_id = _make_shader(GL_FRAGMENT_SHADER, fragment.data.text);
        GLint _log_length;
        GLint _result = GL_FALSE;
        id = glCreateProgram();
        glAttachShader(id, _vertex_id);
        glAttachShader(id, _fragment_id);
        glLinkProgram(id);
        glGetProgramiv(id, GL_LINK_STATUS, &_result);
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &_log_length);
#if defined(LUCARIA_DEBUG)
        if (_log_length > 0) {
            std::vector<GLchar> _result_error_msg(_log_length + 1);
            glGetProgramInfoLog(id, _log_length, NULL, &_result_error_msg[0]);
            std::cout << "While linking program '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
            if (!_result) {
                LUCARIA_DEBUG_ERROR("Failed linking program")
            }
        }
#endif
        glDetachShader(id, _vertex_id);
        glDetachShader(id, _fragment_id);
        glDeleteShader(_vertex_id);
        glDeleteShader(_fragment_id);
        reflected_attributes = _reflect_attributes(id);
        reflected_uniforms = _reflect_uniforms(id);

		ownership.emplace();
    }

    void rendering_program::use() const
    {
        glUseProgram(id);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void rendering_program::bind_attribute(const std::string& name, const rendering_mesh& mesh, const data_vertex_attribute attribute)
    {
        bound_indices_count = mesh.size;
        bound_array_id = mesh.array_id;
        bound_index_offset = mesh.element_offset;
        const std::unordered_map<data_vertex_attribute, uint32>& _attribute_offsets = mesh.attribute_offsets;
        if (reflected_attributes.find(name) == reflected_attributes.end()) {
            LUCARIA_DEBUG_ERROR("Name " + name + " not found in object_shader")
        }
        GLint _location = reflected_attributes.at(name);
        GLuint _size = _mesh_attribute_sizes.at(attribute);
        glBindVertexArray(bound_array_id);
        if (_attribute_offsets.find(attribute) == _attribute_offsets.end()) {
            LUCARIA_DEBUG_ERROR("Attribute " + std::to_string(static_cast<int>(attribute)) + " is not in mesh")
        }
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertices_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.elements_id);
        const void* _offset = reinterpret_cast<const void*>(static_cast<uintptr_t>(mesh.vertex_offset + _attribute_offsets.at(attribute)));
        if (attribute == data_vertex_attribute::bones) {
            glVertexAttribIPointer(_location, _size, GL_INT, mesh.vertex_stride, _offset);
        } else {
            glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, mesh.vertex_stride, _offset);
        }
        glEnableVertexAttribArray(_location);
    }

    void rendering_program::bind_uniform(const std::string& name, const rendering_cubemap& cubemap, const GLuint slot) const
    {
        GLint _location = reflected_uniforms.at(name);
        glUniform1i(_location, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
    }

    void rendering_program::bind_uniform(const std::string& name, const rendering_texture& texture, const GLuint slot) const
    {
        GLint _location = reflected_uniforms.at(name);
        glUniform1i(_location, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture.id);
    }

    template <>
    void rendering_program::bind_uniform<int32>(const std::string& name, const int32& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        glUniform1i(_location, value);
    }

    template <>
    void rendering_program::bind_uniform<float32>(const std::string& name, const float32& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        glUniform1f(_location, value);
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32>>(const std::string& name, const std::vector<float32>& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        const GLuint _count = static_cast<GLuint>(value.size());
        const GLfloat* _ptr = const_cast<const GLfloat*>(value.data());
        glUniform1fv(_location, _count, _ptr);
    }

    template <>
    void rendering_program::bind_uniform<float32x2>(const std::string& name, const float32x2& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        glUniform2f(_location, value.x, value.y);
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x2>>(const std::string& name, const std::vector<float32x2>& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        const GLuint _count = static_cast<GLuint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniform2fv(_location, _count, _ptr);
    }

    template <>
    void rendering_program::bind_uniform<float32x3>(const std::string& name, const float32x3& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        glUniform3f(_location, value.x, value.y, value.z);
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x3>>(const std::string& name, const std::vector<float32x3>& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        const GLuint _count = static_cast<GLuint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniform3fv(_location, _count, _ptr);
    }

    template <>
    void rendering_program::bind_uniform<float32x4>(const std::string& name, const float32x4& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        glUniform4f(_location, value.x, value.y, value.z, value.w);
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x4>>(const std::string& name, const std::vector<float32x4>& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        const GLuint _count = static_cast<GLuint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniform4fv(_location, _count, _ptr);
    }

    // TODO MATRICES

    template <>
    void rendering_program::bind_uniform<float32x4x4>(const std::string& name, const float32x4x4& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(value));
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32x4x4>>(const std::string& name, const std::vector<float32x4x4>& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        const GLuint _count = static_cast<GLuint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
    }

    template <>
    void rendering_program::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
    {
        const GLint _location = reflected_uniforms.at(name);
        const GLuint _count = static_cast<GLuint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
    }

    void rendering_program::draw(const bool use_depth) const
    {
        if (use_depth) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
        }
        glBindVertexArray(bound_array_id);
        glDrawElements(
            GL_TRIANGLES,
            bound_indices_count,
            GL_UNSIGNED_INT,
            reinterpret_cast<const void*>(static_cast<uintptr_t>(bound_index_offset)));
    }

#if defined(LUCARIA_DEBUG)
    void rendering_program::bind_guizmo(const std::string& name, const rendering_mesh_line& from)
    {
        bound_indices_count = from.size;
        bound_array_id = from.array_handle;
        bound_index_offset = 0;
        GLuint _positions_id = from.positions_handle;
        GLint _location = reflected_attributes.at(name);
        GLuint _size = 3;
        glBindVertexArray(bound_array_id);
        glBindBuffer(GL_ARRAY_BUFFER, _positions_id);
        glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(_location);
    }

    void rendering_program::draw_guizmo() const
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glBindVertexArray(bound_array_id);
        glDrawElements(GL_LINES, bound_indices_count, GL_UNSIGNED_INT, 0);
    }
#endif

	void rendering_program::viewport(const uint32x2 size)
	{
		glViewport(0, 0, size.x, size.y);
	}
	
	void rendering_program::clear(const bool clear_depth)
	{
		static bool _is_clear_color_set = false;
		if (!_is_clear_color_set) {
			glClearColor(1.f, 1.f, 1.f, 1.f);
		}
		const GLbitfield _bits = clear_depth ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
		if (clear_depth) {
			glDepthMask(GL_TRUE);
		}
		glClear(_bits);
	}

}

}
