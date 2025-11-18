#pragma once

#include <imgui.h>

#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/viewport.hpp>

// clang-format off
#define IMGUID_CONCAT(lhs, rhs) lhs # rhs
#define IMGUID_CONCAT_WRAPPER(lhs, rhs) IMGUID_CONCAT(lhs, rhs)
#define IMGUID_UNIQUE IMGUID_CONCAT_WRAPPER(__FILE__, __LINE__)
#define IMGUID(NAME) NAME "###" IMGUID_UNIQUE
#define IMGUIDU "###" IMGUID_UNIQUE
// clang-format on

namespace lucaria {
namespace detail {
    struct rendering_system;
}

/// @brief Represents a specialization for interface components
enum struct interface_mode {
    screen,
    spatial
};

/// @brief Represents an interface component for drawing complex UIs on the screen or inside a viewport
/// @tparam mode_t interface_mode value for specialization of interface components
template <interface_mode mode_t>
struct interface_component;

template <>
struct interface_component<interface_mode::screen> {
    interface_component() = default;
    interface_component(const interface_component& other) = delete;
    interface_component& operator=(const interface_component& other) = delete;
    interface_component(interface_component&& other) = default;
    interface_component& operator=(interface_component&& other) = default;

    /// @brief Sets an ImGui callback that will be executed on a correct context
    /// @param callback the user callback to use ImGui from
    /// @return this instance for chaining methods
    interface_component& set_callback(const std::function<void()>& callback);

private:
    std::function<void()> _imgui_callback = nullptr;
    friend struct detail::rendering_system;
};

/// @brief 
enum struct spatial_refresh_mode {
    always,
    never,
    once
};

template <>
struct interface_component<interface_mode::spatial> {
    interface_component();
    interface_component(const interface_component& other) = delete;
    interface_component& operator=(const interface_component& other) = delete;
    interface_component(interface_component&& other);
    interface_component& operator=(interface_component&& other);
    ~interface_component();

    interface_component& use_viewport(viewport& from);
    interface_component& use_viewport(fetched<viewport>& from);
    interface_component& use_interaction_texture(texture& from);
    interface_component& use_interaction_texture(fetched<texture>& from);

    /// @brief Sets an ImGui callback that will be executed on a correct context
    /// @param callback the user callback to use ImGui from
    /// @return this instance for chaining methods
    interface_component& set_callback(const std::function<void()>& callback);
    interface_component& set_refresh(const spatial_refresh_mode mode);
    interface_component& set_interaction(const bool interaction);
    interface_component& set_cursor_size(const glm::vec2& size);

private:
    bool _is_owning = false;
    bool _use_interaction = false;
    detail::fetched_container<viewport> _viewport = {};
    detail::fetched_container<texture> _interaction_texture = {};
    glm::vec2 _cursor_size = { 10, 10 };
    std::function<void()> _imgui_callback = nullptr;
    std::optional<spatial_refresh_mode> _refresh_mode = std::nullopt;
    ImGuiContext* _imgui_context = nullptr;
    std::unique_ptr<texture> _imgui_color_texture = nullptr;
    std::unique_ptr<framebuffer> _imgui_framebuffer = nullptr;
    friend struct detail::rendering_system;
};

/// @brief Specialization for screen interface components
using screen_interface_component = interface_component<interface_mode::screen>;

/// @brief Specialization for spatial interface components
using spatial_interface_component = interface_component<interface_mode::spatial>;

}
