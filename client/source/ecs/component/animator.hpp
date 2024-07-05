#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include <core/animation.hpp>
#include <core/armature.hpp>
#include <core/skeleton.hpp>

/// @brief 
struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    /// @brief 
    /// @param value 
    /// @return 
    animator_component& armature(const armature_data& value);

    /// @brief 
    /// @param name 
    /// @param value 
    /// @return 
    animator_component& animation(const std::string& name, animation_ref&& value);

    /// @brief 
    /// @param value 
    /// @return 
    animator_component& skeleton(skeleton_ref&& value);

private:
    void _update_futures();
    
    std::optional<std::future<armature_data>> _future_armature = std::nullopt;
    std::optional<armature_data> _armature = std::nullopt;
    std::optional<std::future<skeleton_ref>> _future_skeleton = std::nullopt;
    std::optional<skeleton_ref> _skeleton = std::nullopt;
    std::unordered_map<std::string, std::optional<std::future<animation_ref>>> _future_animations = {};
    std::unordered_map<std::string, std::optional<animation_ref>> _animations = {};
    friend struct motion_system;
};