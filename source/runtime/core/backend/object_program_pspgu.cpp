#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/program.hpp>

namespace lucaria {
namespace detail {

    rendering_program::rendering_program(rendering_program&& other)
    {
    }

    rendering_program& rendering_program::operator=(rendering_program&& other)
    {
        return *this;
    }

    rendering_program::~rendering_program()
    {
    }

    rendering_program::rendering_program(const object_shader& vertex, const object_shader& fragment)
    {
    }

    void rendering_program::use() const
    {
    }

    void rendering_program::bind_attribute(const std::string& name, const detail::asset_mesh& mesh, const detail::mesh_attribute attribute)
    {
    }

    void rendering_program::bind_uniform(const std::string& name, const detail::asset_cubemap& cubemap, const glm::uint slot) const
    {
    }

    void rendering_program::bind_uniform(const std::string& name, const detail::asset_texture& texture, const glm::uint slot) const
    {
    }

    template <>
    void rendering_program::bind_uniform<glm::int32>(const std::string& name, const glm::int32& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<float32>(const std::string& name, const float32& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<float32>>(const std::string& name, const std::vector<float32>& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<glm::vec2>(const std::string& name, const glm::vec2& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<glm::vec2>>(const std::string& name, const std::vector<glm::vec2>& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<glm::vec3>(const std::string& name, const glm::vec3& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<glm::vec3>>(const std::string& name, const std::vector<glm::vec3>& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<glm::vec4>(const std::string& name, const glm::vec4& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<glm::vec4>>(const std::string& name, const std::vector<glm::vec4>& value)
    {
    }

    // TODO MATRICES

    template <>
    void rendering_program::bind_uniform<glm::mat4x4>(const std::string& name, const glm::mat4x4& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<std::vector<glm::mat4x4>>(const std::string& name, const std::vector<glm::mat4x4>& value)
    {
    }

    template <>
    void rendering_program::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
    {
    }

    void rendering_program::draw(const bool use_depth) const
    {
    }

#if defined(LUCARIA_DEBUG)
    void rendering_program::bind_guizmo(const std::string& name, const _detail::rendering_mesh_line& from)
    {
    }

    void rendering_program::draw_guizmo() const
    {
    }
#endif

	void rendering_program::viewport(const uint32x2 size)
	{
	}
	
	void rendering_program::clear(const bool clear_depth)
	{
	}

}

}
