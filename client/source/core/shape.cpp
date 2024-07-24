#include <core/fetch.hpp>
#include <core/mesh.hpp>
#include <core/shape.hpp>

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

shape_ref::shape_ref(const geometry_data& data, const shape_type shape)
{
    if (shape == shape_type::box) {
        std::cout << "BOXXXXXXXXXXXXXX \n";
        std::terminate();
    } else if (shape == shape_type::convex_hull) {
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

std::shared_future<std::shared_ptr<shape_ref>> fetch_shape(const std::filesystem::path& geometry_path, const shape_type shape)
{
    std::promise<std::shared_ptr<shape_ref>>& _promise = detail::promises[geometry_path.string()];
    fetch_file(geometry_path, [&_promise, shape](const std::vector<char>& geometry_bytes) {
        _promise.set_value(std::move(std::make_shared<shape_ref>(load_geometry_data(geometry_bytes), shape)));
    });
    return _promise.get_future();
}
