#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace lucaria {
namespace detail {
    struct dynamics_system;
    struct interface_system;
    struct motion_system;
    struct mixer_system;
    struct rendering_system;
}

struct animator_component;

struct transform_component {
    transform_component() = default;
    transform_component(const transform_component& other) = delete;
    transform_component& operator=(const transform_component& other) = delete;
    transform_component(transform_component&& other) = default;
    transform_component& operator=(transform_component&& other) = default;

    transform_component& use_parent(transform_component& parent_transform);
    // transform_component& use_parent(transform_component& parent_transform, animator_component& parent_animator, const glm::uint bone_index);

    transform_component& set_position_relative(const glm::vec3& position);
    transform_component& set_position_warp(const glm::vec3& position);
    transform_component& set_rotation_relative(const glm::vec3& rotation);
    transform_component& set_rotation_warp(const glm::vec3& rotation);
    transform_component& set_transform_relative(const glm::mat4& transform);
    transform_component& set_transform_warp(const glm::mat4& transform);

    [[nodiscard]] glm::vec3 get_position() const;
    [[nodiscard]] glm::quat get_rotation() const;
    [[nodiscard]] glm::vec3 get_right() const;
    [[nodiscard]] glm::vec3 get_up() const;
    [[nodiscard]] glm::vec3 get_forward() const;

private:
    glm::mat4 _transform = glm::mat4(1);
    std::vector<std::reference_wrapper<transform_component>> _children = {};
    void _apply_delta_to_children(const glm::mat4& delta);
    friend struct detail::dynamics_system;
    friend struct detail::interface_system;
    friend struct detail::motion_system;
    friend struct detail::mixer_system;
    friend struct detail::rendering_system;
};

}
