#include <ecs/component/rigidbody.hpp>

rigidbody_component::rigidbody_component(rigidbody_component&& other)
{
    *this = std::move(other);
}

rigidbody_component& rigidbody_component::operator=(rigidbody_component&& other)
{
    return *this;
}

rigidbody_component::~rigidbody_component()
{

}

rigidbody_component& rigidbody_component::box(const glm::vec3& half_extents)
{
    return *this;
}

rigidbody_component& rigidbody_component::capsule(const glm::float32 radius, const glm::float32 height)
{
    return *this;
}
