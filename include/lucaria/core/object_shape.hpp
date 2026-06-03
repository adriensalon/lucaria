#pragma once

#include <memory>

#include <btBulletDynamicsCommon.h>

#include <lucaria/core/object_geometry.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;

    enum struct object_shape_algorithm {
        convex_hull,
        triangle_mesh,
        impact_triangle_mesh
    };

    enum struct object_shape_origin {
        path,
        data,
        box,
        sphere,
        capsule,
        cone
    };

    struct object_shape {
        LUCARIA_DELETE_DEFAULT(object_shape)
        object_shape(const object_shape& other) = delete;
        object_shape& operator=(const object_shape& other) = delete;
        object_shape(object_shape&& other) = default;
        object_shape& operator=(object_shape&& other) = default;

        object_shape(const object_geometry& geometry, const object_shape_algorithm algorithm = object_shape_algorithm::convex_hull);
        object_shape(btCollisionShape* collision_shape, const glm::float32 half_height = 0.f);

        object_shape_origin origin;
		std::optional<std::filesystem::path> origin_path;
        std::optional<float32x3> origin_extents;
        std::optional<object_shape_algorithm> algorithm;
		
        std::unique_ptr<btCollisionShape> collision_shape;
        std::unique_ptr<btTriangleMesh> triangle_geometry;
        glm::mat4 feet_to_center;
        glm::mat4 center_to_feet;
        glm::float32 half_height;
    };

    [[nodiscard]] assets_cell<object_shape>& fetch(
        manager_assets& objects,
        assets_buffer<object_shape>& cached_vector,
        const std::filesystem::path& path,
        const object_shape_algorithm algorithm);

    // recipes

    struct recipe_object_shape_path {
        std::filesystem::path path;
        object_shape_algorithm algorithm;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
            archive(cereal::make_nvp("algorithm", algorithm));
        }
    };

    struct recipe_object_shape_data {
        data_geometry data;
        object_shape_algorithm algorithm;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
            archive(cereal::make_nvp("algorithm", algorithm));
        }
    };

    struct recipe_object_shape_box {
        float32x3 half_extents;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("half_extents", half_extents));
        }
    };

    struct recipe_object_shape_sphere {
        float32 radius;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("radius", radius));
        }
    };

    struct recipe_object_shape_capsule {
        float32 radius;
        float32 height;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("radius", radius));
            archive(cereal::make_nvp("height", height));
        }
    };

    struct recipe_object_shape_cone {
        float32 radius;
        float32 height;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("radius", radius));
            archive(cereal::make_nvp("height", height));
        }
    };

    using recipe_object_shape = std::variant<
        recipe_object_shape_path,
        recipe_object_shape_data,
        recipe_object_shape_box,
        recipe_object_shape_sphere,
        recipe_object_shape_capsule,
        recipe_object_shape_cone>;

    [[nodiscard]] recipe_object_shape make_recipe(const assets_cell<object_shape>& cache);
	[[nodiscard]] assets_cell<object_shape>* apply_recipe(manager_assets& objects, assets_buffer<object_shape>& cached, recipe_object_shape& recipe);

}
}
