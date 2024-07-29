#pragma once

#include <glm/glm.hpp>
#include <ozz/base/maths/simd_math.h>

[[nodiscard]] glm::mat4& reinterpret(ozz::math::Float4x4& matrix);
[[nodiscard]] const glm::mat4& reinterpret(const ozz::math::Float4x4& matrix);
[[nodiscard]] ozz::math::Float4x4& reinterpret_ozz(glm::mat4& matrix);
[[nodiscard]] const ozz::math::Float4x4& reinterpret_ozz(const glm::mat4& matrix);