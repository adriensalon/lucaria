#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <lucaria/public/component_transform.hpp>

namespace lucaria {

void component_transform::_apply_delta_to_children(const float32x4x4& delta)
{
    for (const handle_entity& _child_entity : _children) {
        component_transform& _child = _child_entity.get_transform();
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
    _parent = parent_entity;
    component_transform& _transform = parent_entity.get_transform();
    // _transform._children.emplace_back(std::ref(*this));
    return *this;
}

// component_transform& component_transform::use_parent(component_transform& transform, component_animator& animator, const glm::uint bone_index)
// {
//     animator._children_transforms[bone_index].emplace_back(std::ref(*this));
//     return *this;
// }

component_transform& component_transform::set_position_relative(const float32x3& value)
{
    const float32x4x4 _old_transform = _transform;
    _transform = glm::translate(_transform, value);
    const float32x4x4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_position_warp(const float32x3& value)
{
    const float32x4x4 _old_transform = _transform;
    _transform[3] = float32x4(value, 1.0f);
    const float32x4x4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_rotation_relative(const float32x3& value)
{
    const float32x4x4 _old_transform = _transform;
    _transform = glm::rotate(_transform, value.x, float32x3(1, 0, 0));
    _transform = glm::rotate(_transform, value.y, float32x3(0, 1, 0));
    _transform = glm::rotate(_transform, value.z, float32x3(0, 0, 1));
    const float32x4x4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_rotation_warp(const float32x3& value)
{
    const float32x4x4 _old_transform = _transform;
    const float32x4 _position = _transform[3];
    _transform = float32x4x4(1);
    _transform[3] = _position;
    _transform = glm::rotate(_transform, value.x, float32x3(1, 0, 0));
    _transform = glm::rotate(_transform, value.y, float32x3(0, 1, 0));
    _transform = glm::rotate(_transform, value.z, float32x3(0, 0, 1));
    const float32x4x4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

component_transform& component_transform::set_transform_relative(const float32x4x4& value)
{
    _transform = value * _transform;
    _apply_delta_to_children(value);
    return *this;
}

component_transform& component_transform::set_transform_warp(const float32x4x4& value)
{
    const float32x4x4 _old_transform = _transform;
    _transform = value;
    const float32x4x4 _delta_transform = _transform * glm::inverse(_old_transform);
    _apply_delta_to_children(_delta_transform);
    return *this;
}

float32x3 component_transform::get_position() const
{
    return _transform[3];
}

glm::quat component_transform::get_rotation() const
{
    return glm::quat_cast(_transform);
}

float32x3 component_transform::get_right() const
{
    return glm::normalize(float32x3(_transform[0]));
}

float32x3 component_transform::get_up() const
{
    return glm::normalize(float32x3(_transform[1]));
}

float32x3 component_transform::get_forward() const
{
    return glm::normalize(-float32x3(_transform[2]));
}

}
