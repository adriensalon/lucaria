#pragma once

#include <future>
#include <unordered_map>

#include <core/mesh.hpp>
#include <core/texture.hpp>

enum struct model_texture {
    color,
    normal
};

enum struct model_shader {
    unlit,
    triplanar
};

/// @brief 
struct model_component {
    model_component() = default;
    model_component(const model_component& other) = delete;
    model_component& operator=(const model_component& other) = delete;
    model_component(model_component&& other) = default;
    model_component& operator=(model_component&& other) = default;

    /// @brief 
    /// @param value 
    model_component& mesh(mesh_ref&& value);

    /// @brief 
    /// @param value 
    model_component& mesh(std::future<mesh_ref>&& value);

    /// @brief 
    /// @param type 
    /// @param value 
    model_component& texture(const model_texture type, texture_ref&& value);

    /// @brief 
    /// @param type 
    /// @param value 
    model_component& texture(const model_texture type, std::future<texture_ref>&& value);

private:
    void _update_futures();

    std::optional<std::future<mesh_ref>> _future_mesh = std::nullopt;
    std::optional<mesh_ref> _mesh = std::nullopt;
    std::unordered_map<model_texture, std::optional<std::future<texture_ref>>> _future_textures = {};
    std::unordered_map<model_texture, std::optional<texture_ref>> _textures = {};
    friend struct rendering_system;
};