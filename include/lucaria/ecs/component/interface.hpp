#pragma once

#include <imgui.h>

#include <lucaria/core/font.hpp>
#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/viewport.hpp>

namespace lucaria {
namespace detail {
    struct rendering_system;
}

namespace ecs {

    enum struct interface_mode {
        screen,
        spatial
    };

    template <interface_mode mode_t>
    struct interface_component;

    template <>
    struct interface_component<interface_mode::screen> {
        interface_component() = default;
        interface_component(const interface_component& other) = delete;
        interface_component& operator=(const interface_component& other) = delete;
        interface_component(interface_component&& other) = default;
        interface_component& operator=(interface_component&& other) = default;
        
        interface_component& set_callback(const std::function<void()>& callback);

    private:
        std::function<void()> _imgui_callback = nullptr;
        friend struct detail::rendering_system;
    };

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

        interface_component& set_callback(const std::function<void()>& callback);
        interface_component& set_refresh(const spatial_refresh_mode mode);
        interface_component& set_mouse_position_pixels(const glm::uint x, const glm::uint y);
        interface_component& set_mouse_position_uv(const glm::float32 x, const glm::float32 y);
        interface_component& set_mouse_down(const glm::uint button = 0);
        interface_component& set_mouse_up(const glm::uint button = 0);
        interface_component& set_mouse_click(const glm::uint button = 0, int release_after_frames = 1);
        interface_component& set_scroll(const glm::float32 x, const glm::float32 y);

    private:
        bool _is_owning = false;
        detail::fetched_container<viewport> _viewport = {};
        std::function<void()> _imgui_callback = nullptr;
        std::optional<spatial_refresh_mode> _refresh_mode = std::nullopt;
        ImGuiContext* _imgui_context = nullptr;
        std::unique_ptr<texture> _imgui_color_texture = nullptr;
        std::unique_ptr<framebuffer> _imgui_framebuffer = nullptr;
        friend struct detail::rendering_system;
    };

    using screen_interface_component = interface_component<interface_mode::screen>;
    using spatial_interface_component = interface_component<interface_mode::spatial>;

}
}
