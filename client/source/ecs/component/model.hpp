#pragma once

#include <future>

#include <core/mesh.hpp>
#include <core/texture.hpp>

/// @brief 
struct model_component {

    /// @brief 
    model_component() = delete;

    /// @brief 
    /// @param mesh 
    /// @param color 
    model_component(const mesh_data& mesh, const texture_data& color);

    /// @brief 
    /// @param mesh 
    /// @param color 
    model_component(std::future<mesh_data>&& mesh, std::future<texture_data>&& color);

    /// @brief 
    /// @param other 
    model_component(const model_component& other) = delete;
    
    /// @brief 
    /// @param other 
    /// @return 
    model_component& operator=(const model_component& other) = delete;

    /// @brief 
    /// @param other 
    model_component(model_component&& other) = default;

    /// @brief 
    /// @param other 
    /// @return 
    model_component& operator=(model_component&& other) = default;

    /// @brief When set to false this renderer will not be picked up by the rendering system.
    bool is_enabled = true;

private:
    friend struct rendering_system;
    std::optional<std::future<mesh_data>> _future_mesh = std::nullopt;
    std::optional<std::future<texture_data>> _future_color = std::nullopt;
    std::optional<mesh_ref> _mesh = std::nullopt;
    std::optional<texture_ref> _color = std::nullopt;
};