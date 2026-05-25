#pragma once

#include <imgui.h>

#include <lucaria/core/object_mesh.hpp>
#include <lucaria/core/object_framebuffer.hpp>
#include <lucaria/public/handle_geometry.hpp>
#include <lucaria/public/handle_texture.hpp>

namespace lucaria {
namespace detail {
    struct system_rendering;
}

/// @brief Represents an interface component for drawing complex UIs on the screen or inside a viewport
struct component_interface_screen {
    component_interface_screen() = default;
    component_interface_screen(const component_interface_screen& other) = delete;
    component_interface_screen& operator=(const component_interface_screen& other) = delete;
    component_interface_screen(component_interface_screen&& other) = default;
    component_interface_screen& operator=(component_interface_screen&& other) = default;

    /// @brief Sets an ImGui callback that will be executed on a correct context
    /// @param callback the user callback to use ImGui from
    /// @return this instance for chaining methods
    component_interface_screen& draw_callback(const std::function<void()>& callback);

private:
    std::function<void()> _imgui_callback = nullptr;

    template <typename ArchiveType>
    void serialize(ArchiveType& archive)
    {
        // nothing to serialize
    }

    friend struct detail::system_rendering;
    friend class cereal::access;
};

struct component_interface_spatial {
    component_interface_spatial();
    component_interface_spatial(const component_interface_spatial& other) = delete;
    component_interface_spatial& operator=(const component_interface_spatial& other) = delete;
    component_interface_spatial(component_interface_spatial&& other) = default;
    component_interface_spatial& operator=(component_interface_spatial&& other) = default;
    ~component_interface_spatial();

    component_interface_spatial& use_viewport(const handle_geometry geometry, const glm::uvec2& size);
    component_interface_spatial& use_interaction_texture(const handle_texture texture);
    component_interface_spatial& set_interaction(const bool interaction);
    component_interface_spatial& set_cursor_size(const glm::vec2& size);
    component_interface_spatial& draw_callback(const std::function<void()>& callback);

    [[nodiscard]] std::optional<glm::vec2> get_interaction_position()
    {
        return _interaction_screen_position;
    }

private:
	std::function<void(detail::manager_window&)> _setup_callback = nullptr;
    detail::flag_owning _ownership = {};
    bool _use_interaction = false;
    glm::uvec2 _viewport_size = glm::uvec2(0);
    std::optional<glm::vec2> _interaction_screen_position = std::nullopt;
    handle_texture _interaction_texture = {};
    handle_geometry _viewport_geometry = {};
    std::optional<detail::object_mesh> _viewport_mesh = std::nullopt;
    glm::vec2 _cursor_size = { 10, 10 };
    std::function<void()> _imgui_callback = nullptr;
    ImGuiContext* _imgui_context = nullptr;
    std::optional<detail::object_texture> _imgui_color_texture = std::nullopt;
    std::optional<detail::object_framebuffer> _imgui_framebuffer = std::nullopt;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        archive(cereal::make_nvp("use_interaction", _use_interaction));
        archive(cereal::make_nvp("viewport_size", _viewport_size));
        archive(cereal::make_nvp("interaction_texture", _interaction_texture));
        archive(cereal::make_nvp("viewport_geometry", _viewport_geometry));
        archive(cereal::make_nvp("cursor_size", _cursor_size));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
        archive(cereal::make_nvp("use_interaction", _use_interaction));
        archive(cereal::make_nvp("viewport_size", _viewport_size));
        archive(cereal::make_nvp("interaction_texture", _interaction_texture));
        archive(cereal::make_nvp("viewport_geometry", _viewport_geometry));
        archive(cereal::make_nvp("cursor_size", _cursor_size));
        if (_viewport_geometry) {
            use_viewport(_viewport_geometry, _viewport_size);
        }
        if (_interaction_texture) {
            use_interaction_texture(_interaction_texture);
        }
        set_interaction(_use_interaction);
        set_cursor_size(_cursor_size);
    }

    friend struct detail::system_rendering;
    friend class cereal::access;
};

}
