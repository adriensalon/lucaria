#pragma once

#include <vector>

#include <lucaria/core/math.hpp>

namespace lucaria {

/// @brief 
struct transform_component {
    transform_component() = default;
    transform_component(const transform_component& other) = delete;
    transform_component& operator=(const transform_component& other) = delete;
    transform_component(transform_component&& other) = default;
    transform_component& operator=(transform_component&& other) = default;

    /// @brief 
    /// @param parent_transform 
    /// @return this instance for chaining methods
    transform_component& use_parent(transform_component& parent_transform);

    /// @brief 
    /// @param position 
    /// @return this instance for chaining methods
    transform_component& set_position_relative(const glm::vec3& position);
    
    /// @brief 
    /// @param position 
    /// @return this instance for chaining methods
    transform_component& set_position_warp(const glm::vec3& position);
    
    /// @brief 
    /// @param rotation 
    /// @return this instance for chaining methods
    transform_component& set_rotation_relative(const glm::vec3& rotation);
    
    /// @brief 
    /// @param rotation 
    /// @return this instance for chaining methods
    transform_component& set_rotation_warp(const glm::vec3& rotation);
    
    /// @brief 
    /// @param transform 
    /// @return this instance for chaining methods
    transform_component& set_transform_relative(const glm::mat4& transform);
    
    /// @brief 
    /// @param transform 
    /// @return this instance for chaining methods
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
    friend struct dynamics_system;
    friend struct interface_system;
    friend struct motion_system;
    friend struct mixer_system;
    friend struct rendering_system;
};

}
