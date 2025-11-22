#pragma once

#include <memory>

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>

#include <lucaria/core/geometry.hpp>

namespace lucaria {

/// @brief Algorithms to create shapes from geometry data
enum struct shape_algorithm {
    convex_hull,
    triangle_mesh,
    impact_triangle_mesh
};

/// @brief Represents runtime geometry meant for collision detection on the device
struct shape {
    LUCARIA_DELETE_DEFAULT(shape)
    shape(const shape& other) = delete;
    shape& operator=(const shape& other) = delete;
    shape(shape&& other) = default;
    shape& operator=(shape&& other) = default;

    /// @brief Creates a shape from geometry data
    /// @param from the geometry data to create from
    /// @param algorithm selected algorithm to create shape
    shape(const geometry& from, const shape_algorithm algorithm = shape_algorithm::convex_hull);

    /// @brief Creates a shape from an existing Bullet shape instance
    /// @param handle Bullet shape instance to create from
    /// @param zdistance the half extent along Y axis
    shape(btCollisionShape* handle, const glm::float32 zdistance);

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] btCollisionShape* get_handle();

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] const btCollisionShape* get_handle() const;

    /// @brief Returns a matrix that translates +Y half extent
    /// @return the translation matrix
    [[nodiscard]] glm::mat4 get_feet_to_center() const;

    /// @brief Returns a matrix that translates -Y half extent
    /// @return the translation matrix
    [[nodiscard]] glm::mat4 get_center_to_feet() const;
    
    /// @brief Creates a simple shape from box half extents
    /// @param half_extents half of the shape size in every dimension
    [[nodiscard]] static shape create_box(const glm::vec3& half_extents);

    /// @brief Creates a simple shape from sphere radius
    /// @param radius radius of the shape size in every dimension
    [[nodiscard]] static shape create_sphere(const glm::float32 radius);

    /// @brief Creates a simple shape from capsule radius and height
    /// @param radius radius of the shape size in X and Z dimensions
    /// @param height height of the shape in Y dimension
    [[nodiscard]] static shape create_capsule(const glm::float32 radius, const glm::float32 height);

    /// @brief Creates a simple shape from cone radius and height
    /// @param radius radius of the shape base in X and Z dimensions
    /// @param height height of the shape in Y dimension
    [[nodiscard]] static shape create_cone(const glm::float32 radius, const glm::float32 height);

private:
    std::unique_ptr<btCollisionShape> _handle;
    std::unique_ptr<btTriangleMesh> _triangle_handle;
    glm::mat4 _feet_to_center;
    glm::mat4 _center_to_feet;
    glm::float32 _zdistance;
};

/// @brief Loads geometry from a file asynchronously and uploads directly to the device
/// @param data_path path to load from
/// @param algorithm selected algorithm to create shape
[[nodiscard]] fetched<shape> fetch_shape(const std::filesystem::path& data_path, const shape_algorithm algorithm = shape_algorithm::convex_hull);

}
