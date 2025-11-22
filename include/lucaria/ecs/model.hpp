#pragma once

#include <lucaria/core/mesh.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/texture.hpp>

namespace lucaria {

/// @brief 
struct blockout_model_component {
    blockout_model_component() = default;
    blockout_model_component(const blockout_model_component& other) = delete;
    blockout_model_component& operator=(const blockout_model_component& other) = delete;
    blockout_model_component(blockout_model_component&& other) = default;
    blockout_model_component& operator=(blockout_model_component&& other) = default;

    /// @brief 
    /// @param from 
    /// @return 
    blockout_model_component& use_mesh(mesh& from);

    /// @brief 
    /// @param from 
    /// @return 
    blockout_model_component& use_mesh(fetched<mesh>& from);

private:
    _detail::fetched_container<mesh> _mesh = {};
    friend struct motion_system;
    friend struct rendering_system;
};

/// @brief 
struct unlit_model_component {
    unlit_model_component() = default;
    unlit_model_component(const unlit_model_component& other) = delete;
    unlit_model_component& operator=(const unlit_model_component& other) = delete;
    unlit_model_component(unlit_model_component&& other) = default;
    unlit_model_component& operator=(unlit_model_component&& other) = default;

    /// @brief 
    /// @param from 
    /// @return 
    unlit_model_component& use_mesh(mesh& from);

    /// @brief 
    /// @param from 
    /// @return 
    unlit_model_component& use_mesh(fetched<mesh>& from);

    /// @brief 
    /// @param from 
    /// @return 
    unlit_model_component& use_color(texture& from);

    /// @brief 
    /// @param from 
    /// @return 
    unlit_model_component& use_color(fetched<texture>& from);

private:
    _detail::fetched_container<mesh> _mesh = {};
    _detail::fetched_container<texture> _color = {};
    friend struct motion_system;
    friend struct rendering_system;
};

}
