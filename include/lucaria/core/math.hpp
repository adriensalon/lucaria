#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ozz/base/maths/simd_math.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btTransform.h>

namespace lucaria {
    
/// @brief Projects a vector on a plane
/// @param vector the vector to project
/// @param normal the normal of the plane
/// @return the projected vector
[[nodiscard]] glm::vec3 project_on_plane(const glm::vec3& vector, const glm::vec3& normal);

/// @brief Converts to glm types
/// @param vector the bullet vector to convert
/// @return the converted vector
[[nodiscard]] glm::vec3 convert(const btVector3& vector);

/// @brief Converts to glm types
/// @param vector the ozz vector to convert
/// @return the converted vector
[[nodiscard]] glm::vec3 convert(const ozz::math::Float3& vector);

/// @brief Converts to glm types
/// @param vector the bullet transform to convert
/// @return the converted matrix
[[nodiscard]] glm::mat4 convert(const btTransform& transform);

/// @brief Converts to glm types
/// @param vector the ozz matrix to convert
/// @return the converted matrix
[[nodiscard]] glm::mat4 convert(const ozz::math::Float4x4& matrix);

/// @brief Converts to ozz types
/// @param vector the matrix to convert
/// @return the converted ozz matrix
[[nodiscard]] ozz::math::Float4x4 convert_ozz(const glm::mat4& matrix);

/// @brief Converts to ozz types
/// @param vector the matrix to convert
/// @return the converted ozz matrix
[[nodiscard]] btVector3 convert_bullet(const glm::vec3& vector);
[[nodiscard]] btQuaternion convert_bullet(const glm::quat& vector);
[[nodiscard]] btTransform convert_bullet(const glm::mat4& matrix);

}
