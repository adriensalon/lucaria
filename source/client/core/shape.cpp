#include <lucaria/core/math.hpp>
#include <lucaria/core/shape.hpp>

namespace lucaria {
namespace {

    static void make_convex_hull_shape(const geometry& from, std::unique_ptr<btCollisionShape>& handle)
    {
        handle = std::make_unique<btConvexHullShape>();
        btConvexHullShape* _hull_shape = static_cast<btConvexHullShape*>(handle.get());
        for (const glm::vec3& _position : from.data.positions) {
            _hull_shape->addPoint(btVector3(_position.x, _position.y, _position.z));
        }
        _hull_shape->recalcLocalAabb();
    }

    static void make_triangle_mesh_shape(const geometry& from, std::unique_ptr<btCollisionShape>& handle, std::unique_ptr<btTriangleMesh>& triangle_handle)
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

}

shape::shape(const geometry& from, const shape_algorithm algorithm)
{
    if (algorithm == shape_algorithm::convex_hull) {
        make_convex_hull_shape(from, _handle);

    } else if (algorithm == shape_algorithm::triangle_mesh) {
        make_triangle_mesh_shape(from, _handle, _triangle_handle);
    }

    // todo impact triangle mesh
    _zdistance = 0.f;// NOOOOOOOOOOOOOOOO
}

shape::shape(btCollisionShape* handle, const glm::float32 zdistance)
{
    _handle = std::unique_ptr<btCollisionShape>(handle);
    _zdistance = zdistance;
}

btCollisionShape* shape::get_handle()
{
    return _handle.get();
}

const btCollisionShape* shape::get_handle() const
{
    return _handle.get();
}

glm::float32 shape::get_zdistance() const
{
    return _zdistance;
}

shape create_box_shape(const glm::vec3& half_extents)
{
    return shape(new btBoxShape(detail::reinterpret_bullet(half_extents)), half_extents.z);
}

shape create_sphere_shape(const glm::float32 radius)
{
    return shape(new btSphereShape(static_cast<btScalar>(radius)), radius);
}

shape create_capsule_shape(const glm::float32 radius, const glm::float32 height)
{
    return shape(new btCapsuleShape(
        static_cast<btScalar>(radius),
        static_cast<btScalar>(height)), height * 0.5f);
}

shape create_cone_shape(const glm::float32 radius, const glm::float32 height)
{    
    return shape(new btConeShape(
        static_cast<btScalar>(radius),
        static_cast<btScalar>(height)), height * 0.5f);
}

fetched<shape> fetch_shape(const std::filesystem::path& geometry_data_path, const shape_algorithm algorithm)
{
    std::shared_ptr<std::promise<shape>> _promise = std::make_shared<std::promise<shape>>();

    detail::fetch_bytes(geometry_data_path, [_promise, algorithm](const std::vector<char>& _data_bytes) {
        geometry _geometry(_data_bytes);
        shape _shape(_geometry, algorithm);
        _promise->set_value(std::move(_shape));
    });

    // creating bullet collision shapes on worker thread is safe
    return fetched<shape>(_promise->get_future());
}

}
