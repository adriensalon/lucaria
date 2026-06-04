#pragma once

#include <memory>

#include <btBulletDynamicsCommon.h>

#include <lucaria/core/object_geometry.hpp>
#include <lucaria/core/utils_math.hpp>

#include <lucaria/core/context_serialize.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

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
        object_shape() = default;
        object_shape(const object_shape& other) = delete;
        object_shape& operator=(const object_shape& other) = delete;
        object_shape(object_shape&& other) = default;
        object_shape& operator=(object_shape&& other) = default;

        object_shape(const object_geometry& geometry, const object_shape_algorithm algorithm = object_shape_algorithm::convex_hull);
        object_shape(btCollisionShape* collision_shape, const glm::float32 half_height = 0.f);

        object_shape_origin origin;
		std::filesystem::path origin_path;
        float32x3 origin_extents;
        object_shape_algorithm algorithm;		
        std::unique_ptr<btCollisionShape> collision_shape;
        std::unique_ptr<btTriangleMesh> triangle_geometry;
        glm::mat4 feet_to_center;
        glm::mat4 center_to_feet;
        glm::float32 half_height;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            if (origin == object_shape_origin::path) {
                context.field("origin_path", origin_path);
                context.field("algorithm", algorithm);
            }
            if (origin == object_shape_origin::box || origin == object_shape_origin::sphere || origin == object_shape_origin::capsule || origin == object_shape_origin::cone) {
                context.field("origin_extents", origin_extents);
            }
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            if (origin == object_shape_origin::path) {
                context.field("origin_path", origin_path);
                context.field("algorithm", algorithm);
                const std::filesystem::path _path = origin_path;
                const object_shape_algorithm _algorithm = algorithm;
                context.fetch(_path, [this, _path, _algorithm](const std::vector<char>& bytes) {
                    object_geometry _geometry(bytes);
                    *this = object_shape(_geometry, _algorithm);
                    origin_path = _path;
                });
            } else if (origin == object_shape_origin::box) {
                context.field("origin_extents", origin_extents);
                *this = object_shape(new btBoxShape(convert_bullet(origin_extents)), origin_extents.z);
            } else if (origin == object_shape_origin::sphere) {
                context.field("origin_extents", origin_extents);
                *this = object_shape(new btSphereShape(static_cast<btScalar>(origin_extents.x)), static_cast<btScalar>(origin_extents.x));
            } else if (origin == object_shape_origin::capsule) {
                context.field("origin_extents", origin_extents);
                *this = object_shape(new btCapsuleShape(static_cast<btScalar>(origin_extents.x), static_cast<btScalar>(origin_extents.y)));
            } else if (origin == object_shape_origin::cone) {
                context.field("origin_extents", origin_extents);
                *this = object_shape(new btConeShape(static_cast<btScalar>(origin_extents.x), static_cast<btScalar>(origin_extents.y)));
            }
        }

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
