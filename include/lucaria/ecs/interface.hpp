#pragma once

#include <imgui.h>

#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/mesh.hpp>

namespace lucaria {

/// @brief
enum struct refresh_mode {
    always,
    never,
    once
};

/// @brief Represents an interface component for drawing complex UIs on the screen or inside a viewport
struct screen_interface_component {
    screen_interface_component() = default;
    screen_interface_component(const screen_interface_component& other) = delete;
    screen_interface_component& operator=(const screen_interface_component& other) = delete;
    screen_interface_component(screen_interface_component&& other) = default;
    screen_interface_component& operator=(screen_interface_component&& other) = default;

    /// @brief Sets an ImGui callback that will be executed on a correct context
    /// @param callback the user callback to use ImGui from
    /// @return this instance for chaining methods
    screen_interface_component& set_callback(const std::function<void()>& callback);

    screen_interface_component& set_refresh(const refresh_mode mode);

private:
    std::function<void()> _imgui_callback = nullptr;
    friend struct rendering_system;
};

struct spatial_interface_component {
    spatial_interface_component();
    spatial_interface_component(const spatial_interface_component& other) = delete;
    spatial_interface_component& operator=(const spatial_interface_component& other) = delete;
    spatial_interface_component(spatial_interface_component&& other);
    spatial_interface_component& operator=(spatial_interface_component&& other);
    ~spatial_interface_component();

    spatial_interface_component& use_viewport(geometry& from, const glm::uvec2& size);
    spatial_interface_component& use_viewport(fetched<geometry>& from, const glm::uvec2& size);
    spatial_interface_component& use_interaction_texture(texture& from);
    spatial_interface_component& use_interaction_texture(fetched<texture>& from);

    /// @brief Sets an ImGui callback that will be executed on a correct context
    /// @param callback the user callback to use ImGui from
    /// @return this instance for chaining methods
    spatial_interface_component& set_callback(const std::function<void()>& callback);
    spatial_interface_component& set_refresh(const refresh_mode mode);
    spatial_interface_component& set_interaction(const bool interaction);
    spatial_interface_component& set_cursor_size(const glm::vec2& size);

    [[nodiscard]] std::optional<glm::vec2> get_interaction_position()
    {
        return _interaction_screen_position;
    }

private:
    bool _is_owning = false;
    bool _use_interaction = false;
    glm::uvec2 _viewport_size = glm::uvec2(0);
    std::optional<glm::vec2> _interaction_screen_position = std::nullopt;
    _detail::fetched_container<texture> _interaction_texture = {};
    _detail::fetched_container<geometry> _viewport_geometry = {};
    std::unique_ptr<mesh> _viewport_mesh = nullptr;
    glm::vec2 _cursor_size = { 10, 10 };
    std::function<void()> _imgui_callback = nullptr;
    std::optional<refresh_mode> _refresh_mode = std::nullopt;
    ImGuiContext* _imgui_context = nullptr;
    std::unique_ptr<texture> _imgui_color_texture = nullptr;
    std::unique_ptr<framebuffer> _imgui_framebuffer = nullptr;
    friend struct rendering_system;
};

}
