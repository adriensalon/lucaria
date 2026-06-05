#include <lucaria/core/manager_app.hpp>
#include <lucaria/engine/component_interface.hpp>

namespace lucaria {
namespace {

    static void _invert_texcoords(std::vector<float32x2>& texcoords)
    {
        for (float32x2& _texcoord : texcoords) {
            _texcoord = float32x2(_texcoord.x, 1.0f - _texcoord.y);
        }
    }

}

component_interface_screen& component_interface_screen::draw_callback(const std::function<void()>& callback)
{
    _imgui_callback = callback;
    return *this;
}

component_interface_spatial::component_interface_spatial()
{
    _setup_callback = [this](detail::manager_window& window) {
        _imgui_context = window.create_shared_imgui_context();
    };
	_ownership.emplace();
}

component_interface_spatial::~component_interface_spatial()
{
    if (_ownership.owns()) {
        ImGui::DestroyContext(_imgui_context);
    }
}

component_interface_spatial& component_interface_spatial::use_viewport(const handle_geometry geometry, const uint32x2& size)
{
    _viewport_geometry = geometry;
    _viewport_geometry._cached->fetched.on_ready([this](detail::object_geometry& _on_ready_geometry) {
        _invert_texcoords(_on_ready_geometry.data.texcoords);
        _viewport_mesh.emplace(_viewport_geometry._cached->fetched.value());
    });

    _viewport_size = size;
    _imgui_color_texture.emplace(size);
    _imgui_framebuffer.emplace();
    _imgui_framebuffer->bind_color(_imgui_color_texture.value());
    return *this;
}

component_interface_spatial& component_interface_spatial::use_interaction_texture(const handle_texture texture)
{
    _interaction_texture = texture;
    return *this;
}

component_interface_spatial& component_interface_spatial::draw_callback(const std::function<void()>& callback)
{
    _imgui_callback = callback;
    return *this;
}

component_interface_spatial& component_interface_spatial::set_interaction(const bool interaction)
{
    _use_interaction = interaction;
    return *this;
}

component_interface_spatial& component_interface_spatial::set_cursor_size(const float32x2& cursor_size)
{
    _cursor_size = cursor_size;
    return *this;
}

}