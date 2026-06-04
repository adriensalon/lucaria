#include <glm/gtc/matrix_transform.hpp>

#include <lucaria/core/manager_assets.hpp>
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

        glm::float32 _zdistance = 0.f;
        feet_to_center = glm::translate(float32x4x4(1), float32x3(0, +_zdistance, 0));
        center_to_feet = glm::inverse(feet_to_center);
    }

    object_shape::object_shape(btCollisionShape* handle, const glm::float32 zdistance)
		: origin_extents(float32x3(0))
    {
        const int _shape_type = handle->getShapeType();
        if (_shape_type == BOX_SHAPE_PROXYTYPE) {
            btBoxShape* _casted_shape = static_cast<btBoxShape*>(handle);
            origin = object_shape_origin::box;
            origin_extents = convert(_casted_shape->getHalfExtentsWithoutMargin());
        
		} else if (_shape_type == SPHERE_SHAPE_PROXYTYPE) {
            btSphereShape* _casted_shape = static_cast<btSphereShape*>(handle);
            origin = object_shape_origin::sphere;
            origin_extents.x = static_cast<float32>(_casted_shape->getRadius());
        
		} else if (_shape_type == CAPSULE_SHAPE_PROXYTYPE) {
            btCapsuleShape* _casted_shape = static_cast<btCapsuleShape*>(handle);
            origin = object_shape_origin::capsule;
            origin_extents.x = static_cast<float32>(_casted_shape->getRadius());
            origin_extents.y = 2.f * static_cast<float32>(_casted_shape->getHalfHeight());
        
		} else if (_shape_type == CONE_SHAPE_PROXYTYPE) {
            btConeShape* _casted_shape = static_cast<btConeShape*>(handle);
            origin = object_shape_origin::cone;
            origin_extents.x = static_cast<float32>(_casted_shape->getRadius());
            origin_extents.y = static_cast<float32>(_casted_shape->getHeight());
        
		} else {
            LUCARIA_DEBUG_ERROR("Implementation error");
        }

        collision_shape = std::unique_ptr<btCollisionShape>(handle);
        feet_to_center = glm::translate(float32x4x4(1), float32x3(0, +zdistance, 0));
        center_to_feet = glm::translate(float32x4x4(1), float32x3(0, -zdistance, 0));
    }

}
}
