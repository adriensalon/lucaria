#include <lucaria/core/fetch.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/shape.hpp>

namespace lucaria {
namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<shape_ref>>> promises;


btCollisionShape* make_convex_hull_shape(const geometry_data& data)
{
    btConvexHullShape* _shape = new btConvexHullShape();
    for (const glm::vec3& _position : data.positions) {
        _shape->addPoint(btVector3(_position.x, _position.y, _position.z));
    }
    _shape->recalcLocalAabb();
    return _shape;
}

btCollisionShape* make_triangle_mesh_shape(const geometry_data& data)
{
    btVector3 _vertex_1, _vertex_2, _vertex_3;
    btTriangleMesh* _triangle_mesh = new btTriangleMesh(); // STORE TO DELETE ON UNLOAD()
    for (const glm::uvec3& _index : data.indices) {
        _vertex_1 = btVector3(data.positions[_index.x].x, data.positions[_index.x].y, data.positions[_index.x].z);
        _vertex_2 = btVector3(data.positions[_index.y].x, data.positions[_index.y].y, data.positions[_index.y].z);
        _vertex_3 = btVector3(data.positions[_index.z].x, data.positions[_index.z].y, data.positions[_index.z].z);
        _triangle_mesh->addTriangle(_vertex_1, _vertex_2, _vertex_3);
    }
    btBvhTriangleMeshShape* _shape = new btBvhTriangleMeshShape(_triangle_mesh, true);
    // delete _triangle_mesh;
    return _shape;
}

}

shape_ref::shape_ref(shape_ref&& other)
{
    *this = std::move(other);
}

shape_ref& shape_ref::operator=(shape_ref&& other)
{
    _shape = other._shape;
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

shape_ref::~shape_ref()
{
    if (_is_instanced) {
        delete _shape;
    }
}

shape_ref::shape_ref(const geometry_data& data, const shape_type shape)
{
    if (shape == shape_type::box) {
        std::cout << "BOXXXXXXXXXXXXXX \n";
        std::terminate();

    } else if (shape == shape_type::convex_hull) {
        _shape = detail::make_convex_hull_shape(data);
    } else if (shape == shape_type::triangle_mesh) {
        _shape = detail::make_triangle_mesh_shape(data);
    } // etc
    _is_instanced = true;
}

btCollisionShape* shape_ref::get_shape() const
{
    return _shape;
}

std::shared_future<std::shared_ptr<shape_ref>> fetch_shape(const std::filesystem::path& geometry_path, const shape_type shape)
{
    std::promise<std::shared_ptr<shape_ref>>& _promise = detail::promises[geometry_path.string()];
    fetch_file(geometry_path, [&_promise, shape](const std::vector<char>& geometry_bytes) {
        _promise.set_value(std::make_shared<shape_ref>(load_geometry_data(geometry_bytes), shape));
    });
    return _promise.get_future();
}

void clear_shape_fetches()
{
    detail::promises.clear();
}

}
