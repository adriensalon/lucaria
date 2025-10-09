#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <lucaria/ecs/component/animator.hpp>
#include <lucaria/ecs/component/transform.hpp>

namespace lucaria {
namespace ecs {

    void transform_component::apply_delta_to_children(const glm::mat4& delta)
    {
        for (std::reference_wrapper<transform_component>& _child_reference : _children) {
            transform_component& _child = _child_reference.get();
            _child._transform = delta * _child._transform;
            _child.apply_delta_to_children(delta);
        }
    }

    transform_component& transform_component::use_parent(transform_component& transform)
    {
        transform._children.emplace_back(std::ref(*this));
        return *this;
    }

    // transform_component& transform_component::use_parent(transform_component& transform, animator_component& animator, const glm::uint bone_index)
    // {
    //     animator._children_transforms[bone_index].emplace_back(std::ref(*this));
    //     return *this;
    // }

    transform_component& transform_component::set_position_relative(const glm::vec3& value)
    {
        const glm::mat4 _old_transform = _transform;
        _transform = glm::translate(_transform, value);
        const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
        apply_delta_to_children(_delta_transform);
        return *this;
    }

    transform_component& transform_component::set_position_warp(const glm::vec3& value)
    {
        const glm::mat4 _old_transform = _transform;
        _transform[3] = glm::vec4(value, 1.0f);
        const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
        apply_delta_to_children(_delta_transform);
        return *this;
    }

    transform_component& transform_component::set_rotation_relative(const glm::vec3& value)
    {
        const glm::mat4 _old_transform = _transform;
        _transform = glm::rotate(_transform, value.x, glm::vec3(1.0f, 0.0f, 0.0f));
        _transform = glm::rotate(_transform, value.y, glm::vec3(0.0f, 1.0f, 0.0f));
        _transform = glm::rotate(_transform, value.z, glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
        apply_delta_to_children(_delta_transform);
        return *this;
    }

    transform_component& transform_component::set_rotation_warp(const glm::vec3& value)
    {
        const glm::mat4 _old_transform = _transform;
        const glm::vec4 _position = _transform[3];
        _transform = glm::mat4(1.0f); // reset to identity
        _transform[3] = _position;
        _transform = glm::rotate(_transform, value.x, glm::vec3(1.0f, 0.0f, 0.0f));
        _transform = glm::rotate(_transform, value.y, glm::vec3(0.0f, 1.0f, 0.0f));
        _transform = glm::rotate(_transform, value.z, glm::vec3(0.0f, 0.0f, 1.0f));
        const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
        apply_delta_to_children(_delta_transform);
        return *this;
    }

    transform_component& transform_component::set_transform_relative(const glm::mat4& value)
    {
        _transform = value * _transform;
        apply_delta_to_children(value);
        return *this;
    }

    transform_component& transform_component::set_transform_warp(const glm::mat4& value)
    {
        const glm::mat4 _old_transform = _transform;
        _transform = value;
        const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
        apply_delta_to_children(_delta_transform);
        return *this;
    }

    glm::vec3 transform_component::get_position() const
    {
        return _transform[3];
    }

    glm::vec3 transform_component::get_right() const
    {
        return glm::normalize(glm::vec3(_transform[0]));
    }

    glm::vec3 transform_component::get_up() const
    {
        return glm::normalize(glm::vec3(_transform[1]));
    }

    glm::vec3 transform_component::get_forward() const
    {
        return glm::normalize(-glm::vec3(_transform[2]));
    }

}
}
