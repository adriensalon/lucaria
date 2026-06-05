#pragma once

#include <memory>

#include <btBulletDynamicsCommon.h>

#include <lucaria/core/serialize_context.hpp>
#include <lucaria/core/utils_math.hpp>
#include <lucaria/engine/asset_geometry.hpp>
#include <lucaria/engine/asset_skeleton.hpp>
#include <lucaria/engine/handle_asset.hpp>

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

        object_shape(const asset_geometry& geometry, const object_shape_algorithm algorithm = object_shape_algorithm::convex_hull);
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
                    asset_geometry _geometry(bytes);
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
}

struct handle_shape : handle_asset<detail::object_shape> {
    using handle_asset<detail::object_shape>::handle_asset;
};

}
