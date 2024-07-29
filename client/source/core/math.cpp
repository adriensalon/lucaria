#include <memory>

#include <core/math.hpp>

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

glm::mat4& reinterpret(ozz::math::Float4x4& matrix)
{
    return detail::reinterpret<ozz::math::Float4x4, glm::mat4>(matrix);
}

const glm::mat4& reinterpret(const ozz::math::Float4x4& matrix)
{
    return detail::reinterpret<ozz::math::Float4x4, glm::mat4>(matrix);
}

ozz::math::Float4x4& reinterpret_ozz(glm::mat4& matrix)
{
    return detail::reinterpret<glm::mat4, ozz::math::Float4x4>(matrix);
}

const ozz::math::Float4x4& reinterpret_ozz(const glm::mat4& matrix)
{
    return detail::reinterpret<glm::mat4, ozz::math::Float4x4>(matrix);
}
