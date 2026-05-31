#pragma once

#include <lucaria/public/handle_entity.hpp>

namespace lucaria {

struct context_scene;

/// @brief
struct component_transform {
    component_transform() = default;
    component_transform(const component_transform& other) = delete;
    component_transform& operator=(const component_transform& other) = delete;
    component_transform(component_transform&& other) = default;
    component_transform& operator=(component_transform&& other) = default;

    /// @brief
    /// @param parent_transform
    /// @return this instance for chaining methods
    component_transform& use_parent();

    /// @brief
    /// @param parent_transform
    /// @return this instance for chaining methods
    component_transform& use_parent(context_scene& scenes, handle_entity parent_entity);

    /// @brief
    /// @param position
    /// @return this instance for chaining methods
    component_transform& set_position_relative(const float32x3& position);

    /// @brief
    /// @param position
    /// @return this instance for chaining methods
    component_transform& set_position_warp(const float32x3& position);

    /// @brief
    /// @param rotation
    /// @return this instance for chaining methods
    component_transform& set_rotation_relative(const float32x3& rotation);

    /// @brief
    /// @param rotation
    /// @return this instance for chaining methods
    component_transform& set_rotation_warp(const float32x3& rotation);

    /// @brief
    /// @param transform
    /// @return this instance for chaining methods
    component_transform& set_transform_relative(const float32x4x4& transform);

    /// @brief
    /// @param transform
    /// @return this instance for chaining methods
    component_transform& set_transform_warp(const float32x4x4& transform);

    [[nodiscard]] float32x3 get_position() const;
    [[nodiscard]] float32quat get_rotation() const;
    [[nodiscard]] float32x3 get_right() const;
    [[nodiscard]] float32x3 get_up() const;
    [[nodiscard]] float32x3 get_forward() const;

private:
    float32x4x4 _transform = float32x4x4(1);
    std::optional<handle_entity> _parent = std::nullopt;
    std::vector<handle_entity> _children = {};

    void _apply_delta_to_children(const float32x4x4& delta);

    template <typename Archive>
    void serialize(Archive& archive)
    {
        archive(cereal::make_nvp("transform", _transform));
        archive(cereal::make_nvp("parent", _parent));
        archive(cereal::make_nvp("children", _children));
    }

    friend struct detail::system_dynamics;
    friend struct detail::system_motion;
    friend struct detail::system_mixer;
    friend struct detail::system_rendering;
    friend class cereal::access;
};

}
