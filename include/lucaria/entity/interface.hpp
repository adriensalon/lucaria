#pragma once

#include <cereal/types/unordered_map.hpp>
#include <imgui.h>

#include <lucaria/core/framebuffer.hpp>
#include <lucaria/core/mesh.hpp>
#include <lucaria/core/owning.hpp>

namespace lucaria {
namespace detail {
    struct rendering_system;
}

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
    screen_interface_component& draw_callback(const std::function<void()>& callback);

private:
    std::function<void()> _imgui_callback = nullptr;

    template <typename ArchiveType>
    void serialize(ArchiveType& archive)
    {
        // nothing to serialize
    }

    friend struct detail::rendering_system;
    friend class cereal::access;
};

struct spatial_interface_component {
    spatial_interface_component();
    spatial_interface_component(const spatial_interface_component& other) = delete;
    spatial_interface_component& operator=(const spatial_interface_component& other) = delete;
    spatial_interface_component(spatial_interface_component&& other) = default;
    spatial_interface_component& operator=(spatial_interface_component&& other) = default;
    ~spatial_interface_component();

    spatial_interface_component& use_viewport(const geometry_object geometry, const glm::uvec2& size);
    spatial_interface_component& use_interaction_texture(const texture_object texture);
    spatial_interface_component& set_interaction(const bool interaction);
    spatial_interface_component& set_cursor_size(const glm::vec2& size);
    spatial_interface_component& draw_callback(const std::function<void()>& callback);

    [[nodiscard]] std::optional<glm::vec2> get_interaction_position()
    {
        return _interaction_screen_position;
    }

private:
    detail::owning_flag _ownership = {};
    bool _use_interaction = false;
    glm::uvec2 _viewport_size = glm::uvec2(0);
    std::optional<glm::vec2> _interaction_screen_position = std::nullopt;
    texture_object _interaction_texture = {};
    geometry_object _viewport_geometry = {};
    std::optional<detail::mesh_implementation> _viewport_mesh = std::nullopt;
    glm::vec2 _cursor_size = { 10, 10 };
    std::function<void()> _imgui_callback = nullptr;
    ImGuiContext* _imgui_context = nullptr;
    std::optional<detail::texture_implementation> _imgui_color_texture = std::nullopt;
    std::optional<detail::framebuffer_implementation> _imgui_framebuffer = std::nullopt;

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

    friend struct detail::rendering_system;
    friend class cereal::access;
};

}
