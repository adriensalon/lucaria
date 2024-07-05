#pragma once

#include <unordered_map>

#include <core/mesh.hpp>
#include <core/texture.hpp>

/// @brief
enum struct model_shader {
    blockout,
    unlit,
    pbr
};

/// @brief
enum struct model_texture {
    color,
    normal,
    occlusion,
    roughness,
    metallic
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
    std::optional<std::future<mesh_ref>> _future_mesh = std::nullopt;
    std::optional<mesh_ref> _mesh = std::nullopt;
    std::unordered_map<model_texture, std::optional<std::future<texture_ref>>> _future_textures = {};
    std::unordered_map<model_texture, std::optional<texture_ref>> _textures = {};
    friend struct async_system;
    friend struct motion_system;
    friend struct rendering_system;
};