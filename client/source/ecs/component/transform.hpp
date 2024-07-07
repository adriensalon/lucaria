#pragma once

#include <optional>

#include <glm/glm.hpp>

struct transform_component {
    transform_component() = default;
    transform_component(const transform_component& other) = delete;
    transform_component& operator=(const transform_component& other) = delete;
    transform_component(transform_component&& other) = default;
    transform_component& operator=(transform_component&& other) = default;

    transform_component& transform_relative(const glm::mat4& value);

    transform_component& transform_warp(const glm::mat4& value);

    transform_component& position_relative(const glm::vec3& value);

    transform_component& position_warp(const glm::vec3& value);

    transform_component& rotation_relative(const glm::vec3& value);

    transform_component& rotation_warp(const glm::vec3& value);

    transform_component& parent(transform_component& value);

private:
    glm::mat4 _transform = glm::mat4(1.0f);
    std::optional<std::reference_wrapper<transform_component>> _parent = std::nullopt;
    friend struct dynamics_system;
    friend struct motion_system;
    friend struct rendering_system;
};