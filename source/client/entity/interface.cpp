#include <lucaria/entity/interface.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {
namespace {

    static void invert_texcoords(std::vector<glm::vec2>& texcoords)
    {
        for (glm::vec2& _texcoord : texcoords) {
            _texcoord = glm::vec2(_texcoord.x, 1.0f - _texcoord.y);
        }
    }

}

screen_interface_component& screen_interface_component::set_callback(const std::function<void()>& callback)
{
    _imgui_callback = callback;
    return *this;
}

spatial_interface_component::spatial_interface_component()
{
    _imgui_context = detail::engine_window().create_shared_imgui_context();
    _is_owning = true;
}

spatial_interface_component::spatial_interface_component(spatial_interface_component&& other)
{
    *this = std::move(other);
}

spatial_interface_component& spatial_interface_component::operator=(spatial_interface_component&& other)
{
    _is_owning = true;
    _viewport_size = std::move(other._viewport_size);
    _viewport_geometry = std::move(other._viewport_geometry);
    _viewport_mesh = std::move(other._viewport_mesh);
    _imgui_callback = std::move(other._imgui_callback);
    _refresh_mode = other._refresh_mode;
    _imgui_context = other._imgui_context;
    _imgui_color_texture = std::move(other._imgui_color_texture);
    _imgui_framebuffer = std::move(other._imgui_framebuffer);
    other._is_owning = false;
    return *this;
}

spatial_interface_component::~spatial_interface_component()
{
    if (_is_owning) {
        ImGui::DestroyContext(_imgui_context);
    }
}

spatial_interface_component& spatial_interface_component::use_viewport(const geometry_object geometry, const glm::uvec2& size)
{
    _viewport_geometry = geometry;
	_viewport_geometry._resource->on_ready([this](detail::geometry_implementation& _on_ready_geometry) {
        invert_texcoords(_on_ready_geometry.data.texcoords);
        _viewport_mesh.emplace(_viewport_geometry._resource->get());
    });

    _viewport_size = size;
    _imgui_color_texture.emplace(size);
    _imgui_framebuffer.emplace();
    _imgui_framebuffer->bind_color(_imgui_color_texture.value());
    return *this;
}

spatial_interface_component& spatial_interface_component::use_interaction_texture(const texture_object texture)
{
    _interaction_texture = texture;
    return *this;
}

spatial_interface_component& spatial_interface_component::set_callback(const std::function<void()>& callback)
{
    _imgui_callback = callback;
    return *this;
}

spatial_interface_component& spatial_interface_component::set_refresh(const refresh_mode mode)
{
    _refresh_mode = mode;
    return *this;
}
spatial_interface_component& spatial_interface_component::set_interaction(const bool interaction)
{
    _use_interaction = interaction;
    return *this;
}

spatial_interface_component& spatial_interface_component::set_cursor_size(const glm::vec2& cursor_size)
{
    _cursor_size = cursor_size;
    return *this;
}

}