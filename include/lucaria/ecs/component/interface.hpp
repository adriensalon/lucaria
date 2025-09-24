#pragma once

#include <functional>
#include <future>
#include <memory>

#include <imgui.h>

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/font.hpp>
#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/texture.hpp>
#include <lucaria/core/world.hpp>

namespace lucaria {

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

    interface_component& font(const std::shared_future<std::shared_ptr<font_ref>>& fetched_font);
    interface_component& callback(const std::function<void(const std::vector<ImFont*>&)>& imgui_callback);

private:
    fetch_container<mesh_ref> _mesh = {};
    std::function<void(const std::vector<ImFont*>&)> _imgui_callback = nullptr;
    ImGuiContext* _imgui_context = nullptr;
    std::vector<ImFont*> _imgui_fonts = {};
    std::unique_ptr<texture_ref> _imgui_color_texture = nullptr;
    std::unique_ptr<framebuffer_ref> _imgui_framebuffer = nullptr;
    bool _is_imgui_input_enabled = true;
    bool _is_imgui_backend_ready = false;
    friend struct rendering_system;
};

template <>
struct interface_component<interface_mode::spatial> {
    interface_component() = default;
    interface_component(const interface_component& other) = delete;
    interface_component& operator=(const interface_component& other) = delete;
    interface_component(interface_component&& other) = default;
    interface_component& operator=(interface_component&& other) = default;

    interface_component& size(const glm::uint width, const glm::uint height);
    interface_component& mesh(const std::shared_future<std::shared_ptr<mesh_ref>>& fetched_mesh);
    interface_component& font(const std::shared_future<std::shared_ptr<font_ref>>& fetched_font, const bool clear = false);
    interface_component& callback(const std::function<void(const std::vector<ImFont*>&)>& imgui_callback);
    interface_component& mouse_position_pixels(const glm::uint x, const glm::uint y);
    interface_component& mouse_position_uv(const glm::float32 x, const glm::float32 y);
    interface_component& mouse_down(const glm::uint button = 0);
    interface_component& mouse_up(const glm::uint button = 0);
    interface_component& mouse_click(const glm::uint button = 0, int release_after_frames = 1);
    interface_component& scroll(const glm::float32 x, const glm::float32 y);
    interface_component& enable_input(const bool enable);

private:
    fetch_container<mesh_ref> _mesh = {};
    std::function<void(const std::vector<ImFont*>&)> _imgui_callback = nullptr;
    ImGuiContext* _imgui_context = nullptr;
    std::vector<ImFont*> _imgui_fonts = {};
    std::unique_ptr<texture_ref> _imgui_color_texture = nullptr;
    std::unique_ptr<framebuffer_ref> _imgui_framebuffer = nullptr;
    bool _is_imgui_input_enabled = true;
    bool _is_imgui_backend_ready = false;
    friend struct rendering_system;
};

using screen_interface_component = interface_component<interface_mode::screen>;
using spatial_interface_component = interface_component<interface_mode::spatial>;

}
