#pragma once

#include <memory>

#include <btBulletDynamicsCommon.h>

#include <lucaria/core/geometry.hpp>

namespace lucaria {
namespace detail {

    struct motion_system;
    struct dynamics_system;

    enum struct shape_algorithm {
        convex_hull,
        triangle_mesh,
        impact_triangle_mesh
    };

    enum struct shape_origin {
        path,
        data,
        box,
        sphere,
        capsule,
        cone
    };

    struct shape_implementation {
        LUCARIA_DELETE_DEFAULT(shape_implementation)
        shape_implementation(const shape_implementation& other) = delete;
        shape_implementation& operator=(const shape_implementation& other) = delete;
        shape_implementation(shape_implementation&& other) = default;
        shape_implementation& operator=(shape_implementation&& other) = default;

        shape_implementation(const geometry_implementation& geometry, const shape_algorithm algorithm = shape_algorithm::convex_hull);
        shape_implementation(btCollisionShape* collision_shape, const glm::float32 half_height = 0.f);

        shape_origin origin;
        std::optional<shape_algorithm> algorithm;
        std::unique_ptr<btCollisionShape> collision_shape;
        std::unique_ptr<btTriangleMesh> triangle_geometry;
        glm::mat4 feet_to_center;
        glm::mat4 center_to_feet;
        glm::float32 half_height;
    };

    struct shape_path_recipe {
        std::filesystem::path path;
        shape_algorithm algorithm;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
            archive(cereal::make_nvp("algorithm", algorithm));
        }
    };

    struct shape_data_recipe {
        geometry_data data;
        shape_algorithm algorithm;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
            archive(cereal::make_nvp("algorithm", algorithm));
        }
    };

    struct shape_box_recipe {
        float32x3 half_extents;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("half_extents", half_extents));
        }
    };

    struct shape_sphere_recipe {
        float32 radius;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("radius", radius));
        }
    };

    struct shape_capsule_recipe {
        float32 radius;
        float32 height;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("radius", radius));
            archive(cereal::make_nvp("height", height));
        }
    };

    struct shape_cone_recipe {
        float32 radius;
        float32 height;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("radius", radius));
            archive(cereal::make_nvp("height", height));
        }
    };

    using shape_recipe = std::variant<
        shape_path_recipe,
        shape_data_recipe,
        shape_box_recipe,
        shape_sphere_recipe,
        shape_capsule_recipe,
        shape_cone_recipe>;

    [[nodiscard]] shape_recipe make_recipe(const implementation_container<shape_implementation>& container);

}

/// @brief Represents runtime geometry meant for collision detection on the device
struct shape_object {
    shape_object() = default;
    shape_object(const shape_object& other) = default;
    shape_object& operator=(const shape_object& other) = default;
    shape_object(shape_object&& other) = default;
    shape_object& operator=(shape_object&& other) = default;

    /// @brief Creates a shape from geometry data
    /// @param geometry the geometry data to create from
    /// @param algorithm selected algorithm to create shape
    static shape_object create(const geometry_object geometry, const detail::shape_algorithm algorithm = detail::shape_algorithm::convex_hull);

    static shape_object create_box(const glm::vec3& half_extents);

    static shape_object create_sphere(const glm::float32 radius);

    static shape_object create_capsule(const glm::float32 radius, const glm::float32 height);

    static shape_object create_cone(const glm::float32 radius, const glm::float32 height);

    static shape_object fetch(const std::filesystem::path& path, const detail::shape_algorithm algorithm = detail::shape_algorithm::convex_hull);

    [[nodiscard]] bool has_value() const;

    [[nodiscard]] explicit operator bool() const;

private:
    detail::refcount_flag _refcount = {};
    detail::implementation_manager<detail::shape_implementation>* _manager = nullptr;
    detail::implementation_container<detail::shape_implementation>* _resource = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

    friend struct detail::motion_system;
    friend struct detail::dynamics_system;
    friend struct passive_rigidbody_component;
    friend struct kinematic_rigidbody_component;
    friend struct dynamic_rigidbody_component;
    friend class cereal::access;
};

}
