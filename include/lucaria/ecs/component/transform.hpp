#pragma once

#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include <lucaria/core/world.hpp>

struct animator_component;

struct transform_component {
    transform_component() = default;
    transform_component(const transform_component& other) = delete;
    transform_component& operator=(const transform_component& other) = delete;
    transform_component(transform_component&& other) = default;
    transform_component& operator=(transform_component&& other) = default;

    transform_component& parent(transform_component& parent_transform);
    transform_component& parent(transform_component& parent_transform, animator_component& parent_animator, const glm::uint bone_index);
    transform_component& position_relative(const glm::vec3& position);
    transform_component& position_warp(const glm::vec3& position);
    transform_component& rotation_relative(const glm::vec3& rotation);
    transform_component& rotation_warp(const glm::vec3& rotation);
    transform_component& transform_relative(const glm::mat4& transform);
    transform_component& transform_warp(const glm::mat4& transform);

    glm::vec3 get_position() const;
    glm::vec3 get_forward() const;
    glm::quat get_rotation() const;
    glm::vec3 get_euler() const;

private:
    glm::mat4 _transform = glm::mat4(1.0f);
    std::vector<std::reference_wrapper<transform_component>> _children = {};
    friend struct dynamics_system;
    friend struct interface_system;
    friend struct motion_system;
    friend struct mixer_system;
    friend struct rendering_system;
};