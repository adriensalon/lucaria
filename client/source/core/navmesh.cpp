#include <core/fetch.hpp>
#include <core/navmesh.hpp>


namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<shape_ref>>> promises;

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

template <>
shape_ref::shape_ref(const shape_data<shape_type::box>& data)
{
}

template <>
shape_ref::shape_ref(const shape_data<shape_type::sphere>& data)
{
}

template <>
shape_ref::shape_ref(const shape_data<shape_type::capsule>& data)
{
}

template <>
shape_ref::shape_ref(const shape_data<shape_type::cylinder>& data)
{
}

template <>
shape_ref::shape_ref(const shape_data<shape_type::cone>& data)
{
}

template <shape_type shape_t>
shape_ref::shape_ref(const geometry_data& data)
{
    if constexpr (shape_t == shape_type::convex_hull) {
        btConvexHullShape* _hull_shape = new btConvexHullShape();
        for (const glm::vec3& _position : data.positions) {
            _hull_shape->addPoint(btVector3(_position.x, _position.y, _position.z));
        }
        _hull_shape->recalcLocalAabb();
        _shape = _hull_shape;
    } // etc
    _is_instanced = true;
}

btCollisionShape* shape_ref::get_shape() const
{
    return _shape;
}

std::shared_future<std::shared_ptr<shape_ref>> fetch_shape(const std::filesystem::path& mesh_path)
{
    std::promise<std::shared_ptr<shape_ref>>& _promise = detail::promises[mesh_path.string()];
    fetch_file(mesh_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<shape_ref>(load_mesh_data(stream))));
    });
    return _promise.get_future();
}
