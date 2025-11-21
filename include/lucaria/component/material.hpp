#pragma once

#include <lucaria/core/mesh.hpp>
#include <lucaria/core/program.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/core/texture.hpp>

namespace lucaria {
namespace detail {
    struct rendering_system;
}
}

namespace lucaria {

/// @brief 
struct blockout_material_component {
    blockout_material_component() = default;
    blockout_material_component(const blockout_material_component& other) = delete;
    blockout_material_component& operator=(const blockout_material_component& other) = delete;
    blockout_material_component(blockout_material_component&& other) = default;
    blockout_material_component& operator=(blockout_material_component&& other) = default;

private:
    friend struct detail::rendering_system;
};

/// @brief 
struct unlit_material_component {
    unlit_material_component() = default;
    unlit_material_component(const unlit_material_component& other) = delete;
    unlit_material_component& operator=(const unlit_material_component& other) = delete;
    unlit_material_component(unlit_material_component&& other) = default;
    unlit_material_component& operator=(unlit_material_component&& other) = default;

    /// @brief 
    /// @param from 
    /// @return 
    unlit_material_component& use_color(texture& from);

    /// @brief 
    /// @param from 
    /// @return 
    unlit_material_component& use_color(fetched<texture>& from);

private:
    detail::fetched_container<texture> _color = {};
    friend struct detail::rendering_system;
};

/// @brief 
struct program_material_component {
    program_material_component() = default;
    program_material_component(const program_material_component& other) = delete;
    program_material_component& operator=(const program_material_component& other) = delete;
    program_material_component(program_material_component&& other) = default;
    program_material_component& operator=(program_material_component&& other) = default;

    /// @brief 
    /// @param from 
    /// @return 
    program_material_component& use_program(program& from);
    
    /// @brief 
    /// @param from 
    /// @return 
    program_material_component& use_program(fetched<program>& from);

    /// @brief 
    /// @param callback 
    void set_draw_callback(const std::function<void(program&)>& callback);
    
    /// @brief 
    /// @param callback 
    void set_draw_callback(const std::function<void(program&, const mesh&)>& callback);
    
    /// @brief 
    /// @param callback 
    void set_draw_callback(const std::function<void(program&, const mesh&, const skeleton&)>& callback);

private:
    detail::fetched_container<program> _program = {};
    std::function<void(program&)> _callback_1 = nullptr;
    std::function<void(program&, const mesh&)> _callback_2 = nullptr;
    std::function<void(program&, const mesh&, const skeleton&)> _callback_3 = nullptr;
    friend struct detail::rendering_system;
};

}