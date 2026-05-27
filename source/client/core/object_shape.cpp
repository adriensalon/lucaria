#include <glm/gtc/matrix_transform.hpp>

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/object_shape.hpp>
#include <lucaria/core/utils_math.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    namespace {

        static void _make_convex_hull_shape(const object_geometry& from, std::unique_ptr<btCollisionShape>& handle)
        {
            handle = std::make_unique<btConvexHullShape>();
            btConvexHullShape* _hull_shape = static_cast<btConvexHullShape*>(handle.get());
            for (const float32x3& _position : from.data.positions) {
                _hull_shape->addPoint(btVector3(_position.x, _position.y, _position.z));
            }
            _hull_shape->recalcLocalAabb();
        }

        static void _make_triangle_mesh_shape(const object_geometry& from, std::unique_ptr<btCollisionShape>& handle, std::unique_ptr<btTriangleMesh>& triangle_handle)
        {
            btVector3 _vertex_1, _vertex_2, _vertex_3;
            triangle_handle = std::make_unique<btTriangleMesh>();
            for (const glm::uvec3& _index : from.data.indices) {
                _vertex_1 = btVector3(from.data.positions[_index.x].x, from.data.positions[_index.x].y, from.data.positions[_index.x].z);
                _vertex_2 = btVector3(from.data.positions[_index.y].x, from.data.positions[_index.y].y, from.data.positions[_index.y].z);
                _vertex_3 = btVector3(from.data.positions[_index.z].x, from.data.positions[_index.z].y, from.data.positions[_index.z].z);
                triangle_handle->addTriangle(_vertex_1, _vertex_2, _vertex_3);
            }
            handle = std::make_unique<btBvhTriangleMeshShape>(triangle_handle.get(), true);
        }

        // todo impact triangle mesh

        static container_async<object_shape> _fetch_shape_async(manager_object& objects, const std::filesystem::path& path, const object_shape_algorithm algorithm)
        {
            std::shared_ptr<std::promise<object_shape>> _promise = std::make_shared<std::promise<object_shape>>();
            objects.fetch_bytes(path, [_promise, algorithm](const std::vector<char>& _data_bytes) {
				object_geometry _geometry(_data_bytes);
				object_shape _shape(_geometry, algorithm);
				_promise->set_value(std::move(_shape)); }, true);

            // creating bullet collision shapes on worker thread is safe
            return container_async<object_shape>(_promise->get_future());
        }

    }

    object_shape::object_shape(const object_geometry& from, const object_shape_algorithm algorithm)
        : origin(from.origin == object_geometry_origin::path ? object_shape_origin::path : object_shape_origin::data)
        , algorithm(algorithm)
    {
        if (algorithm == object_shape_algorithm::convex_hull) {
            _make_convex_hull_shape(from, collision_shape);

        } else if (algorithm == object_shape_algorithm::triangle_mesh) {
            _make_triangle_mesh_shape(from, collision_shape, triangle_geometry);

        } else if (algorithm == object_shape_algorithm::impact_triangle_mesh) {
            LUCARIA_DEBUG_ERROR("Impact triangle mesh not implemented") // TODO
        }

        glm::float32 _zdistance = 0.f; // lets compute ?
        feet_to_center = glm::translate(float32x4x4(1), float32x3(0, +_zdistance, 0));
        center_to_feet = glm::inverse(feet_to_center);
    }

    object_shape::object_shape(btCollisionShape* handle, const glm::float32 zdistance)
        : algorithm(std::nullopt)
    {
        const int _shape_type = handle->getShapeType();
        if (_shape_type == BOX_SHAPE_PROXYTYPE) {
            origin = object_shape_origin::box;
        } else if (_shape_type == SPHERE_SHAPE_PROXYTYPE) {
            origin = object_shape_origin::sphere;
        } else if (_shape_type == CAPSULE_SHAPE_PROXYTYPE) {
            origin = object_shape_origin::capsule;
        } else if (_shape_type == CONE_SHAPE_PROXYTYPE) {
            origin = object_shape_origin::cone;
        } else {
            LUCARIA_DEBUG_ERROR("Implementation error");
        }

        collision_shape = std::unique_ptr<btCollisionShape>(handle);
        feet_to_center = glm::translate(float32x4x4(1), float32x3(0, +zdistance, 0));
        center_to_feet = glm::translate(float32x4x4(1), float32x3(0, -zdistance, 0));
    }

    container_cache<object_shape>& fetch(
        manager_object& objects,
        container_cache_vector<object_shape>& cached_vector,
        const std::filesystem::path& path,
        const object_shape_algorithm algorithm)
    {
        return *cached_vector.get_or_create_by_path(path, [&objects, path, algorithm] {
            return _fetch_shape_async(objects, path, algorithm);
        });
    }

    recipe_object_shape make_recipe(const container_cache<object_shape>& cached)
    {
        const object_shape& _shape = cached.fetched.value();

        if (_shape.origin == object_shape_origin::path) {
            return recipe_object_shape_path { cached.origin_path.value(), _shape.algorithm.value() };

        } else if (_shape.origin == object_shape_origin::data) {
            return {}; // TODO
            // return recipe_object_shape_data { object_geometry(_shape).data, _shape.algorithm.value() };

        } else if (_shape.origin == object_shape_origin::box) {
            btBoxShape* _casted_shape = static_cast<btBoxShape*>(_shape.collision_shape.get());
            return recipe_object_shape_box {
                convert(_casted_shape->getHalfExtentsWithoutMargin())
            };

        } else if (_shape.origin == object_shape_origin::sphere) {
            btSphereShape* _casted_shape = static_cast<btSphereShape*>(_shape.collision_shape.get());
            return recipe_object_shape_sphere {
                static_cast<float32>(_casted_shape->getRadius())
            };

        } else if (_shape.origin == object_shape_origin::capsule) {
            btCapsuleShape* _casted_shape = static_cast<btCapsuleShape*>(_shape.collision_shape.get());
            return recipe_object_shape_capsule {
                static_cast<float32>(_casted_shape->getRadius()),
                2.f * static_cast<float32>(_casted_shape->getHalfHeight())
            };

        } else if (_shape.origin == object_shape_origin::cone) {
            btConeShape* _casted_shape = static_cast<btConeShape*>(_shape.collision_shape.get());
            return recipe_object_shape_cone {
                static_cast<float32>(_casted_shape->getRadius()),
                static_cast<float32>(_casted_shape->getHeight())
            };
        }

        else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    container_cache<object_shape>* apply_recipe(manager_object& objects, container_cache_vector<object_shape>& cached_vector, recipe_object_shape& recipe)
    {
        return std::visit([&](auto& value) -> container_cache<object_shape>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_shape_path>) {
                return &fetch(objects, cached_vector, value.path, value.algorithm);

                // } else if constexpr (std::is_same_v<RecipeType, recipe_object_shape_data>) {
                //     return cached_vector.create_cell(
                //         container_async<object_shape>(
                //             object_shape(std::move(value.data))));

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_shape_box>) {
                return cached_vector.create_cell(
                    container_async<object_shape>(
                        object_shape(
                            new btBoxShape(convert_bullet(value.half_extents)), value.half_extents.z)));

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_shape_sphere>) {
                return cached_vector.create_cell(
                    container_async<object_shape>(
                        object_shape(
                            new btSphereShape(static_cast<btScalar>(value.radius)), static_cast<btScalar>(value.radius))));

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_shape_capsule>) {
                return cached_vector.create_cell(
                    container_async<object_shape>(
                        object_shape(
                            new btCapsuleShape(static_cast<btScalar>(value.radius), static_cast<btScalar>(value.height)))));

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_shape_cone>) {
                return cached_vector.create_cell(
                    container_async<object_shape>(
                        object_shape(
                            new btConeShape(static_cast<btScalar>(value.radius), static_cast<btScalar>(value.height)))));

            } else {
                LUCARIA_DEBUG_ERROR("Implementation error");
                return nullptr;
            }
        },
            recipe);
    }

}
}
