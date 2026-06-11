#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/program.hpp>

namespace lucaria {
namespace detail {

    object_program::object_program(object_program&& other)
    {
    }

    object_program& object_program::operator=(object_program&& other)
    {
        return *this;
    }

    object_program::~object_program()
    {
    }

    object_program::object_program(const object_shader& vertex, const object_shader& fragment)
    {
    }

    void object_program::use() const
    {
    }

    void object_program::bind_attribute(const std::string& name, const detail::object_mesh& mesh, const detail::mesh_attribute attribute)
    {
    }

    void object_program::bind_uniform(const std::string& name, const detail::object_cubemap& cubemap, const glm::uint slot) const
    {
    }

    void object_program::bind_uniform(const std::string& name, const detail::object_texture& texture, const glm::uint slot) const
    {
    }

    template <>
    void object_program::bind_uniform<glm::int32>(const std::string& name, const glm::int32& value)
    {
    }

    template <>
    void object_program::bind_uniform<float32>(const std::string& name, const float32& value)
    {
    }

    template <>
    void object_program::bind_uniform<std::vector<float32>>(const std::string& name, const std::vector<float32>& value)
    {
    }

    template <>
    void object_program::bind_uniform<glm::vec2>(const std::string& name, const glm::vec2& value)
    {
    }

    template <>
    void object_program::bind_uniform<std::vector<glm::vec2>>(const std::string& name, const std::vector<glm::vec2>& value)
    {
    }

    template <>
    void object_program::bind_uniform<glm::vec3>(const std::string& name, const glm::vec3& value)
    {
    }

    template <>
    void object_program::bind_uniform<std::vector<glm::vec3>>(const std::string& name, const std::vector<glm::vec3>& value)
    {
    }

    template <>
    void object_program::bind_uniform<glm::vec4>(const std::string& name, const glm::vec4& value)
    {
    }

    template <>
    void object_program::bind_uniform<std::vector<glm::vec4>>(const std::string& name, const std::vector<glm::vec4>& value)
    {
    }

    // TODO MATRICES

    template <>
    void object_program::bind_uniform<glm::mat4x4>(const std::string& name, const glm::mat4x4& value)
    {
    }

    template <>
    void object_program::bind_uniform<std::vector<glm::mat4x4>>(const std::string& name, const std::vector<glm::mat4x4>& value)
    {
    }

    template <>
    void object_program::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
    {
    }

    void object_program::draw(const bool use_depth) const
    {
    }

#if defined(LUCARIA_DEBUG)
    void object_program::bind_guizmo(const std::string& name, const _detail::object_mesh_line& from)
    {
    }

    void object_program::draw_guizmo() const
    {
    }
#endif

	void object_program::viewport(const uint32x2 size)
	{
	}
	
	void object_program::clear(const bool clear_depth)
	{
	}

}

}
