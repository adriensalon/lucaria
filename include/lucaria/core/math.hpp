#pragma once

#include <glm/glm.hpp>
#include <ozz/base/maths/simd_math.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btTransform.h>

namespace lucaria {
namespace detail {

[[nodiscard]] glm::vec3& reinterpret(btVector3& vector);
[[nodiscard]] const glm::vec3& reinterpret(const btVector3& vector);
[[nodiscard]] glm::vec3& reinterpret(ozz::math::Float3& vector);
[[nodiscard]] const glm::vec3& reinterpret(const ozz::math::Float3& vector);
[[nodiscard]] glm::mat4& reinterpret(ozz::math::Float4x4& matrix);
[[nodiscard]] const glm::mat4& reinterpret(const ozz::math::Float4x4& matrix);
[[nodiscard]] glm::mat4 convert(const btTransform& transform);

[[nodiscard]] ozz::math::Float4x4& reinterpret_ozz(glm::mat4& matrix);
[[nodiscard]] const ozz::math::Float4x4& reinterpret_ozz(const glm::mat4& matrix);

[[nodiscard]] btVector3& reinterpret_bullet(glm::vec3& vector);
[[nodiscard]] const btVector3& reinterpret_bullet(const glm::vec3& vector);
[[nodiscard]] btTransform convert_bullet(const glm::mat4& matrix);

}
}
