#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <lucaria/public/component_transform.hpp>

namespace lucaria {

void component_transform::_apply_delta_to_children(const glm::mat4& delta)
{
    for (std::reference_wrapper<component_transform>& _child_reference : _children) {
        component_transform& _child = _child_reference.get();
        _child._transform = delta * _child._transform;
        _child._apply_delta_to_children(delta);
    }
}

component_transform& component_transform::use_parent()
{
    // if (_parent) {
    //     std::optional<std::size_t> _found_index = std::nullopt;
    //     std::size_t _index = 0;
    //     for (std::reference_wrapper<component_transform>& _child_reference : _parent->get()._children) {
    //         component_transform& _child = _child_reference.get();
    //         if (&_child == this) {
    //             _found_index = _index;
    //             break;
    //         }
    //         _index++;
    //     }
    //     if (_found_index) {
    //         _parent->get()._children.erase(_parent->get()._children.begin() + _found_index.value());
    //     }
    // }
    return *this;
}

component_transform& component_transform::use_parent(handle_entity parent_entity)
{
    _parent_entity = parent_entity;
    component_transform& _transform = parent_entity._registry->get<component_transform>(parent_entity._entity);
    _transform._children.emplace_back(std::ref(*this));
    _parent_transform = std::ref(_transform);
    return *this;
}

// component_transform& component_transform::use_parent(component_transform& transform, component_animator& animator, const glm::uint bone_index)
// {
//     animator._children_transforms[bone_index].emplace_back(std::ref(*this));
//     return *this;
// }

component_transform& component_transform::set_position_relative(const glm::vec3& value)
{
    const glm::mat4 _old_transform = _transform;
    _transform = glm::translate(_transform, value);
    const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_position_warp(const glm::vec3& value)
{
    const glm::mat4 _old_transform = _transform;
    _transform[3] = glm::vec4(value, 1.0f);
    const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_rotation_relative(const glm::vec3& value)
{
    const glm::mat4 _old_transform = _transform;
    _transform = glm::rotate(_transform, value.x, glm::vec3(1, 0, 0));
    _transform = glm::rotate(_transform, value.y, glm::vec3(0, 1, 0));
    _transform = glm::rotate(_transform, value.z, glm::vec3(0, 0, 1));
    const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_rotation_warp(const glm::vec3& value)
{
    const glm::mat4 _old_transform = _transform;
    const glm::vec4 _position = _transform[3];
    _transform = glm::mat4(1);
    _transform[3] = _position;
    _transform = glm::rotate(_transform, value.x, glm::vec3(1, 0, 0));
    _transform = glm::rotate(_transform, value.y, glm::vec3(0, 1, 0));
    _transform = glm::rotate(_transform, value.z, glm::vec3(0, 0, 1));
    const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_transform_relative(const glm::mat4& value)
{
    _transform = value * _transform;
    _apply_delta_to_children(value);
    return *this;
}

component_transform& component_transform::set_transform_warp(const glm::mat4& value)
{
    const glm::mat4 _old_transform = _transform;
    _transform = value;
    const glm::mat4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

glm::vec3 component_transform::get_position() const
{
    return _transform[3];
}

glm::quat component_transform::get_rotation() const
{
    return glm::quat_cast(_transform);
}

glm::vec3 component_transform::get_right() const
{
    return glm::normalize(glm::vec3(_transform[0]));
}

glm::vec3 component_transform::get_up() const
{
    return glm::normalize(glm::vec3(_transform[1]));
}

glm::vec3 component_transform::get_forward() const
{
    return glm::normalize(-glm::vec3(_transform[2]));
}

}
