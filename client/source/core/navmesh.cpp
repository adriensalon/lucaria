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
    btConvexHullShape* hullShape = new btConvexHullShape();
    for (const glm::vec3& vertex : data.positions) {
        hullShape->addPoint(btVector3(vertex.x, vertex.y, vertex.z));
    }
    hullShape->recalcLocalAabb();    
    _shape = hullShape;
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
