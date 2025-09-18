#include <memory>

#include <lucaria/core/math.hpp>

namespace detail {

template <typename from_t, typename to_t>
to_t& reinterpret(from_t& from)
{
    return *(reinterpret_cast<to_t*>(std::addressof(from)));
}

template <typename from_t, typename to_t>
const to_t& reinterpret(const from_t& from)
{
    return *(reinterpret_cast<const to_t*>(std::addressof(from)));
}

}

// to glm

glm::mat4& reinterpret(ozz::math::Float4x4& matrix)
{
    return detail::reinterpret<ozz::math::Float4x4, glm::mat4>(matrix);
}

const glm::mat4& reinterpret(const ozz::math::Float4x4& matrix)
{
    return detail::reinterpret<ozz::math::Float4x4, glm::mat4>(matrix);
}

// to ozz

ozz::math::Float4x4& reinterpret_ozz(glm::mat4& matrix)
{
    return detail::reinterpret<glm::mat4, ozz::math::Float4x4>(matrix);
}

const ozz::math::Float4x4& reinterpret_ozz(const glm::mat4& matrix)
{
    return detail::reinterpret<glm::mat4, ozz::math::Float4x4>(matrix);
}

// to bullet

btVector3& reinterpret_bullet(glm::vec3& vector)
{
    return detail::reinterpret<glm::vec3, btVector3>(vector);
}

const btVector3& reinterpret_bullet(const glm::vec3& vector)
{
    return detail::reinterpret<glm::vec3, btVector3>(vector);
}

