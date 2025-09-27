#pragma once

#include <memory>

#include <btBulletDynamicsCommon.h>

#include <lucaria/core/geometry.hpp>

namespace lucaria {

enum struct shape_algorithm {
    convex_hull,
    triangle_mesh,
    impact_triangle_mesh
};

struct shape {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(shape)
    shape(const shape& other) = delete;
    shape& operator=(const shape& other) = delete;
    shape(shape&& other) = default;
    shape& operator=(shape&& other) = default;

    /// @brief
    /// @param from
    /// @param algorithm
    shape(const geometry& from, const shape_algorithm algorithm = shape_algorithm::convex_hull);

    /// @brief
    /// @param handle
    shape(btCollisionShape* handle, const glm::float32 zdistance);

    [[nodiscard]] btCollisionShape* get_handle();
    [[nodiscard]] const btCollisionShape* get_handle() const;
    [[nodiscard]] glm::float32 get_zdistance() const;

private:
    std::unique_ptr<btCollisionShape> _handle;
    std::unique_ptr<btTriangleMesh> _triangle_handle;
    glm::float32 _zdistance;
};

/// @brief
/// @param half_extents
[[nodiscard]] shape create_box_shape(const glm::vec3& half_extents);

/// @brief
/// @param radius
[[nodiscard]] shape create_sphere_shape(const glm::float32 radius);

/// @brief
/// @param radius
/// @param height
[[nodiscard]] shape create_capsule_shape(const glm::float32 radius, const glm::float32 height);

/// @brief
/// @param radius
/// @param height
[[nodiscard]] shape create_cone_shape(const glm::float32 radius, const glm::float32 height);

/// @brief
/// @param geometry_data_path
/// @param algorithm
[[nodiscard]] fetched<shape> fetch_shape(const std::filesystem::path& geometry_data_path, const shape_algorithm algorithm = shape_algorithm::convex_hull);

}
