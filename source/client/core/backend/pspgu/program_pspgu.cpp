#include <glm/gtc/type_ptr.hpp>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/core/program.hpp>

namespace lucaria {
namespace detail {

    program_implementation::program_implementation(program_implementation&& other)
    {
    }

    program_implementation& program_implementation::operator=(program_implementation&& other)
    {
        return *this;
    }

    program_implementation::~program_implementation()
    {
    }

    program_implementation::program_implementation(const shader& vertex, const shader& fragment)
    {
    }

    void program_implementation::use() const
    {
    }

    void program_implementation::bind_attribute(const std::string& name, const detail::mesh_implementation& mesh, const detail::mesh_attribute attribute)
    {
    }

    void program_implementation::bind_uniform(const std::string& name, const detail::cubemap_implementation& cubemap, const glm::uint slot) const
    {
    }

    void program_implementation::bind_uniform(const std::string& name, const detail::texture_implementation& texture, const glm::uint slot) const
    {
    }

    template <>
    void program_implementation::bind_uniform<glm::int32>(const std::string& name, const glm::int32& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<float32>(const std::string& name, const float32& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<std::vector<float32>>(const std::string& name, const std::vector<float32>& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<glm::vec2>(const std::string& name, const glm::vec2& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::vec2>>(const std::string& name, const std::vector<glm::vec2>& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<glm::vec3>(const std::string& name, const glm::vec3& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::vec3>>(const std::string& name, const std::vector<glm::vec3>& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<glm::vec4>(const std::string& name, const glm::vec4& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::vec4>>(const std::string& name, const std::vector<glm::vec4>& value)
    {
    }

    // TODO MATRICES

    template <>
    void program_implementation::bind_uniform<glm::mat4x4>(const std::string& name, const glm::mat4x4& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<std::vector<glm::mat4x4>>(const std::string& name, const std::vector<glm::mat4x4>& value)
    {
    }

    template <>
    void program_implementation::bind_uniform<ozz::vector<ozz::math::Float4x4>>(const std::string& name, const ozz::vector<ozz::math::Float4x4>& value)
    {
    }

    void program_implementation::draw(const bool use_depth) const
    {
    }

#if defined(LUCARIA_DEBUG)
    void program_implementation::bind_guizmo(const std::string& name, const _detail::guizmo_mesh& from)
    {
    }

    void program_implementation::draw_guizmo() const
    {
    }
#endif

	void program_implementation::viewport(const uint32x2 size)
	{
	}
	
	void program_implementation::clear(const bool clear_depth)
	{
	}

}

}
