#pragma once

#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>
#include <LinearMath/btVector3.h>
#include <imgui.h>
#include <ozz/base/maths/simd_math.h>

#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {

    [[nodiscard]] float32x3 project_on_plane(const float32x3& vector, const float32x3& normal);

    [[nodiscard]] float32x3 convert(const btVector3& vector);
    [[nodiscard]] float32x3 convert(const ozz::math::Float3& vector);
    [[nodiscard]] float32x4x4 convert(const btTransform& transform);
    [[nodiscard]] float32x4x4 convert(const ozz::math::Float4x4& matrix);

    [[nodiscard]] ozz::math::Float4x4 convert_ozz(const float32x4x4& matrix);
    [[nodiscard]] btVector3 convert_bullet(const float32x3& vector);
    [[nodiscard]] btQuaternion convert_bullet(const glm::quat& vector);
    [[nodiscard]] btTransform convert_bullet(const float32x4x4& matrix);
    [[nodiscard]] ImVec2 convert_imgui(const float32x2& vector);

}
}
