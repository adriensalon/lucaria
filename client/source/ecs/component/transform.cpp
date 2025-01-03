#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/transform.hpp>

transform_component& transform_component::parent(transform_component& transform)
{
    transform._children.emplace_back(std::ref(*this));
    return *this;
}

transform_component& transform_component::parent(animator_component& animator, const glm::uint bone_index)
{
    animator._children_transforms[bone_index].emplace_back(std::ref(*this));
    return *this;
}

transform_component& transform_component::position_relative(const glm::vec3& value)
{
    _transform = glm::translate(_transform, value);
    // apply transform to children
    return *this;
}

transform_component& transform_component::position_warp(const glm::vec3& value)
{
    _transform[3] = glm::vec4(value, 1.0f);
    // apply transform to children
    return *this;
}

transform_component& transform_component::rotation_relative(const glm::vec3& value)
{
    _transform = glm::rotate(_transform, value.x, glm::vec3(1.0f, 0.0f, 0.0f));
    _transform = glm::rotate(_transform, value.y, glm::vec3(0.0f, 1.0f, 0.0f));
    _transform = glm::rotate(_transform, value.z, glm::vec3(0.0f, 0.0f, 1.0f));
    // apply transform to children
    return *this;
}

transform_component& transform_component::rotation_warp(const glm::vec3& value)
{
    _transform = glm::mat4(1.0f); // Reset to identity
    _transform = glm::rotate(_transform, value.x, glm::vec3(1.0f, 0.0f, 0.0f));
    _transform = glm::rotate(_transform, value.y, glm::vec3(0.0f, 1.0f, 0.0f));
    _transform = glm::rotate(_transform, value.z, glm::vec3(0.0f, 0.0f, 1.0f));
    // apply transform to children
    return *this;
}

transform_component& transform_component::transform_relative(const glm::mat4& value)
{
    _transform = value * _transform;
    // apply transform to children
    return *this;
}

transform_component& transform_component::transform_warp(const glm::mat4& value)
{
    _transform = value;
    // apply transform to children
    return *this;
}
