#pragma once

#include <optional>
#include <vector>

#include <glm/glm.hpp>

struct transform_component {
    transform_component() = default;
    transform_component(const transform_component& other) = delete;
    transform_component& operator=(const transform_component& other) = delete;
    transform_component(transform_component&& other) = default;
    transform_component& operator=(transform_component&& other) = default;

    transform_component& motion_bone_index(const std::optional<glm::uint> bone_index);
    transform_component& parent(transform_component& parent_component);
    transform_component& position_relative(const glm::vec3& position);
    transform_component& position_warp(const glm::vec3& position);
    transform_component& rotation_relative(const glm::vec3& rotation);
    transform_component& rotation_warp(const glm::vec3& rotation);
    transform_component& transform_relative(const glm::mat4& transform);
    transform_component& transform_warp(const glm::mat4& transform);

private:
    glm::mat4 _transform = glm::mat4(1.0f);
    std::optional<std::reference_wrapper<transform_component>> _parent = std::nullopt; // remove only children
    std::vector<std::reference_wrapper<transform_component>> _children = {};
    glm::uint _motion_bone_index = 0;
    friend struct dynamics_system;
    friend struct motion_system;
    friend struct rendering_system;
};