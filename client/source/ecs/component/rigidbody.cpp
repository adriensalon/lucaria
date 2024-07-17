#include <ecs/component/rigidbody.hpp>

rigidbody_component::rigidbody_component(rigidbody_component&& other)
{
    *this = std::move(other);
}

rigidbody_component& rigidbody_component::operator=(rigidbody_component&& other)
{
    _shape = other._shape;
    _state = other._state;
    _rigidbody = other._rigidbody;
    _is_instanced = other._is_instanced;
    other._is_instanced = false;
    return *this;
}

rigidbody_component::~rigidbody_component()
{
    if (_is_instanced) {
        delete _rigidbody->getMotionState();
        delete _rigidbody;
        delete _shape;
    }
}

rigidbody_component& rigidbody_component::box(const glm::vec3& half_extents)
{
    // TODO
    _is_instanced = true;
    return *this;
}

rigidbody_component& rigidbody_component::capsule(const glm::float32 radius, const glm::float32 height)
{
    // TODO
    _is_instanced = true;
    return *this;
}
