#pragma once

#include <optional>
#include <vector>

#include <lucaria/core/math.hpp>
#include <lucaria/entity/entity.hpp>

namespace lucaria {
namespace detail {
    struct dynamics_system;
    struct motion_system;
    struct mixer_system;
    struct rendering_system;
}

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
    transform_component& use_parent();

    /// @brief
    /// @param parent_transform
    /// @return this instance for chaining methods
    transform_component& use_parent(scene_entity parent_entity);

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
    std::optional<scene_entity> _parent_entity = std::nullopt;
    std::optional<std::reference_wrapper<transform_component>> _parent_transform = {};
    std::vector<std::reference_wrapper<transform_component>> _children = {};

    void _apply_delta_to_children(const glm::mat4& delta);

    template <typename Archive>
    void serialize(Archive& archive)
    {
		archive(cereal::make_nvp("transform", _transform));
        archive(cereal::make_nvp("parent", _parent_entity));
    }

    friend struct detail::dynamics_system;
    friend struct detail::motion_system;
    friend struct detail::mixer_system;
    friend struct detail::rendering_system;
    friend class cereal::access;
};

}
