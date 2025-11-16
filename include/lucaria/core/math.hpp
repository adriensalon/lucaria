#pragma once

#include <glm/glm.hpp>
#include <ozz/base/maths/simd_math.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btTransform.h>

namespace lucaria {
namespace detail {

[[nodiscard]] glm::vec3 convert(const btVector3& vector);
[[nodiscard]] glm::vec3 convert(const ozz::math::Float3& vector);
[[nodiscard]] glm::mat4 convert(const btTransform& transform);
[[nodiscard]] glm::mat4 convert(const ozz::math::Float4x4& matrix);

[[nodiscard]] ozz::math::Float4x4 convert_ozz(const glm::mat4& matrix);

[[nodiscard]] btVector3 convert_bullet(const glm::vec3& vector);
[[nodiscard]] btTransform convert_bullet(const glm::mat4& matrix);

}
}
