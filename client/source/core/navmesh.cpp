#include <core/navmesh.hpp>
#include <core/fetch.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<navmesh_ref>>> promises;

}

navmesh_ref::navmesh_ref(navmesh_ref&& other)
{
    *this = std::move(other);
}

navmesh_ref& navmesh_ref::operator=(navmesh_ref&& other)
{
    _shape = other._shape;
#if LUCARIA_GUIZMO
    _guizmo = std::move(other._guizmo);
#endif
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

navmesh_ref::~navmesh_ref()
{
    if (_is_instanced) {
        delete _shape;
    }
}

navmesh_ref::navmesh_ref(const mesh_data& data)
{
    btTriangleMesh* _triangle_mesh = new btTriangleMesh();
    for (const glm::uvec3& _index : data.indices) {
        btVector3 _vertex_0(data.positions[_index.x].x, data.positions[_index.x].y, data.positions[_index.x].z);
        btVector3 _vertex_1(data.positions[_index.y].x, data.positions[_index.y].y, data.positions[_index.y].z);
        btVector3 _vertex_2(data.positions[_index.z].x, data.positions[_index.z].y, data.positions[_index.z].z);
        _triangle_mesh->addTriangle(_vertex_0, _vertex_1, _vertex_2);
    }
    _shape = new btBvhTriangleMeshShape(_triangle_mesh, true);
#if LUCARIA_GUIZMO
    _guizmo = std::make_unique<guizmo_mesh_ref>(data);
#endif
    _is_instanced = true;
}

btCollisionShape* navmesh_ref::get_shape() const
{
    return _shape;
}

std::shared_future<std::shared_ptr<navmesh_ref>> fetch_navmesh(const std::filesystem::path& mesh_path)
{
    std::promise<std::shared_ptr<navmesh_ref>>& _promise = detail::promises[mesh_path.string()];
    fetch_file(mesh_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<navmesh_ref>(load_mesh_data(stream))));
    });
    return _promise.get_future();
}
