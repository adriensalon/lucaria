#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/program.hpp>

namespace lucaria {

void _fetch_bytes(const std::vector<std::filesystem::path>& file_paths, const std::function<void(const std::vector<std::vector<char>>&)>& callback, bool persist);

namespace detail {
    namespace {

        static const std::unordered_map<detail::mesh_attribute, glm::uint> _mesh_attribute_sizes = {
            { detail::mesh_attribute::position, 3 },
            { detail::mesh_attribute::color, 3 },
            { detail::mesh_attribute::normal, 3 },
            { detail::mesh_attribute::tangent, 3 },
            { detail::mesh_attribute::bitangent, 3 },
            { detail::mesh_attribute::texcoord, 2 },
            { detail::mesh_attribute::bones, 4 },
            { detail::mesh_attribute::weights, 4 },
        };

        [[nodiscard]] static glm::uint _make_shader(const GLenum type, const std::string& text)
        {
            glm::int32 _log_length;
            glm::int32 _result = GL_FALSE;
            glm::uint _shader_id = glCreateShader(type);
            const GLchar* _source_ptr = text.c_str();
            glShaderSource(_shader_id, 1, &_source_ptr, NULL);
            glCompileShader(_shader_id);
            glGetShaderiv(_shader_id, GL_COMPILE_STATUS, &_result);
            glGetShaderiv(_shader_id, GL_INFO_LOG_LENGTH, &_log_length);
#if defined(LUCARIA_DEBUG)
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

        [[nodiscard]] static std::unordered_map<std::string, glm::int32> _reflect_attributes(const glm::uint program_id)
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
// #if defined(LUCARIA_DEBUG)
//                 std::cout << "Program has attribute '" << _name << "' at location " << _location << std::endl;
// #endif
            }
            return _attributes;
        }

        [[nodiscard]] static std::unordered_map<std::string, glm::int32> _reflect_uniforms(const glm::uint program_id)
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
// #if defined(LUCARIA_DEBUG)
//                 std::cout << "Program has uniform '" << _name << "' at location " << _location << std::endl;
// #endif
            }
            return _uniforms;
        }

    }

    program_implementation::program_implementation(program_implementation&& other)
    {
        implementation_opengl.is_owning = false;
        *this = std::move(other);
    }

    program_implementation& program_implementation::operator=(program_implementation&& other)
    {
        if (implementation_opengl.is_owning) {
            LUCARIA_RUNTIME_ERROR("Object already owning resources")
        }

        implementation_opengl.is_owning = other.implementation_opengl.is_owning;
        implementation_opengl.id = other.implementation_opengl.id;
        implementation_opengl.reflected_attributes = std::move(other.implementation_opengl.reflected_attributes);
        implementation_opengl.reflected_uniforms = std::move(other.implementation_opengl.reflected_uniforms);
        implementation_opengl.bound_array_id = other.implementation_opengl.bound_array_id;
        implementation_opengl.bound_indices_count = other.implementation_opengl.bound_indices_count;
        
        other.implementation_opengl.is_owning = false;
        return *this;
    }

    program_implementation::~program_implementation()
    {
        if (implementation_opengl.is_owning) {
            glUseProgram(0);
            glDeleteProgram(implementation_opengl.id);
        }
    }

    program_implementation::program_implementation(const shader& vertex, const shader& fragment)
    {
        glm::uint _vertex_id = _make_shader(GL_VERTEX_SHADER, vertex.data.text);
        glm::uint _fragment_id = _make_shader(GL_FRAGMENT_SHADER, fragment.data.text);
        glm::int32 _log_length;
        glm::int32 _result = GL_FALSE;
        implementation_opengl.id = glCreateProgram();
        glAttachShader(implementation_opengl.id, _vertex_id);
        glAttachShader(implementation_opengl.id, _fragment_id);
        glLinkProgram(implementation_opengl.id);
        glGetProgramiv(implementation_opengl.id, GL_LINK_STATUS, &_result);
        glGetProgramiv(implementation_opengl.id, GL_INFO_LOG_LENGTH, &_log_length);
#if defined(LUCARIA_DEBUG)
        if (_log_length > 0) {
            std::vector<GLchar> _result_error_msg(_log_length + 1);
            glGetProgramInfoLog(implementation_opengl.id, _log_length, NULL, &_result_error_msg[0]);
            std::cout << "While linking program '" << std::string(&_result_error_msg[0]) << "'" << std::endl;
            if (!_result) {
                LUCARIA_RUNTIME_ERROR("Failed linking program")
            }
        }
#endif
        glDetachShader(implementation_opengl.id, _vertex_id);
        glDetachShader(implementation_opengl.id, _fragment_id);
        glDeleteShader(_vertex_id);
        glDeleteShader(_fragment_id);
        implementation_opengl.reflected_attributes = _reflect_attributes(implementation_opengl.id);
        implementation_opengl.reflected_uniforms = _reflect_uniforms(implementation_opengl.id);
    }

    void program_implementation::use() const
    {
        glUseProgram(implementation_opengl.id);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void program_implementation::bind_attribute(const std::string& name, const detail::mesh_implementation& mesh, const detail::mesh_attribute attribute)
    {
        implementation_opengl.bound_indices_count = mesh.size;
        implementation_opengl.bound_array_id = mesh.implementation_opengl.array_id;
        const std::unordered_map<detail::mesh_attribute, glm::uint>& _attribute_handles = mesh.implementation_opengl.attribute_ids;
        if (implementation_opengl.reflected_attributes.find(name) == implementation_opengl.reflected_attributes.end()) {
            LUCARIA_RUNTIME_ERROR("Name " + name + " not found in shader")
        }
        glm::int32 _location = implementation_opengl.reflected_attributes.at(name);
        glm::uint _size = _mesh_attribute_sizes.at(attribute);
        glBindVertexArray(implementation_opengl.bound_array_id);
        if (_attribute_handles.find(attribute) == _attribute_handles.end()) {
            LUCARIA_RUNTIME_ERROR("Attribute " + std::to_string(static_cast<int>(attribute)) + " is not in mesh")
        }
        glBindBuffer(GL_ARRAY_BUFFER, _attribute_handles.at(attribute));
        if (attribute == detail::mesh_attribute::bones) {
            glVertexAttribIPointer(_location, _size, GL_INT, _size * sizeof(glm::int32), (void*)0);
        } else {
            glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(glm::float32), (void*)0);
        }
        glEnableVertexAttribArray(_location);
    }

    void program_implementation::bind_uniform(const std::string& name, const detail::cubemap_implementation& cubemap, const glm::uint slot) const
    {
        glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniform1i(_location, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.implementation_opengl.id);
    }

    void program_implementation::bind_uniform(const std::string& name, const detail::texture_implementation& texture, const glm::uint slot) const
    {
        glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniform1i(_location, slot);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture.implementation_opengl.id);
    }

    template <>
    void program_implementation::bind_uniform<glm::int32>(const std::string& name, const glm::int32& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniform1i(_location, value);
    }

    template <>
    void program_implementation::bind_uniform<GLfloat>(const std::string& name, const GLfloat& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniform1f(_location, value);
    }

    template <>
    void program_implementation::bind_uniform<std::vector<GLfloat>>(const std::string& name, const std::vector<GLfloat>& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        const glm::uint _count = static_cast<glm::uint>(value.size());
        const GLfloat* _ptr = const_cast<const GLfloat*>(value.data());
        glUniform1fv(_location, _count, _ptr);
    }

    template <>
    void program_implementation::bind_uniform<glm::vec2>(const std::string& name, const glm::vec2& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniform2f(_location, value.x, value.y);
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::vec2>>(const std::string& name, const std::vector<glm::vec2>& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        const glm::uint _count = static_cast<glm::uint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniform2fv(_location, _count, _ptr);
    }

    template <>
    void program_implementation::bind_uniform<glm::vec3>(const std::string& name, const glm::vec3& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniform3f(_location, value.x, value.y, value.z);
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::vec3>>(const std::string& name, const std::vector<glm::vec3>& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        const glm::uint _count = static_cast<glm::uint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniform3fv(_location, _count, _ptr);
    }

    template <>
    void program_implementation::bind_uniform<glm::vec4>(const std::string& name, const glm::vec4& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniform4f(_location, value.x, value.y, value.z, value.w);
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::vec4>>(const std::string& name, const std::vector<glm::vec4>& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        const glm::uint _count = static_cast<glm::uint>(value.size());
        const GLfloat* _ptr = reinterpret_cast<const GLfloat*>(value.data());
        glUniform4fv(_location, _count, _ptr);
    }

    // TODO MATRICES

    template <>
    void program_implementation::bind_uniform<glm::mat4x4>(const std::string& name, const glm::mat4x4& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        glUniformMatrix4fv(_location, 1, GL_FALSE, glm::value_ptr(value));
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::mat4x4>>(const std::string& name, const std::vector<glm::mat4x4>& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        const glm::uint _count = static_cast<glm::uint>(value.size());
        const glm::float32* _ptr = reinterpret_cast<const glm::float32*>(value.data());
        glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
    }

    template <>
    void program_implementation::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
    {
        const glm::int32 _location = implementation_opengl.reflected_uniforms.at(name);
        const glm::uint _count = static_cast<glm::uint>(value.size());
        const glm::float32* _ptr = reinterpret_cast<const glm::float32*>(value.data());
        glUniformMatrix4fv(_location, _count, GL_FALSE, _ptr);
    }

    void program_implementation::draw(const bool use_depth) const
    {
        if (use_depth) {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
        } else {
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
        }
        glBindVertexArray(implementation_opengl.bound_array_id);
        glDrawElements(GL_TRIANGLES, implementation_opengl.bound_indices_count, GL_UNSIGNED_INT, 0);
    }

#if defined(LUCARIA_DEBUG)
    void program_implementation::bind_guizmo(const std::string& name, const _detail::guizmo_mesh& from)
    {
        implementation_opengl.bound_indices_count = from.get_size();
        implementation_opengl.bound_array_id = from.get_array_handle();
        glm::uint _positions_id = from.get_positions_handle();
        glm::int32 _location = implementation_opengl.reflected_attributes.at(name);
        glm::uint _size = 3;
        glBindVertexArray(implementation_opengl.bound_array_id);
        glBindBuffer(GL_ARRAY_BUFFER, _positions_id);
        glVertexAttribPointer(_location, _size, GL_FLOAT, GL_FALSE, _size * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(_location);
    }

    void program_implementation::draw_guizmo() const
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glBindVertexArray(implementation_opengl.bound_array_id);
        glDrawElements(GL_LINES, implementation_opengl.bound_indices_count, GL_UNSIGNED_INT, 0);
    }
#endif

	void program_implementation::viewport(const uint32x2 size)
	{
		glViewport(0, 0, size.x, size.y);
	}
	
	void program_implementation::clear(const bool clear_depth)
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
