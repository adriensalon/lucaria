#include <algorithm>

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

#if defined(LUCARIA_BACKEND_PSPGU)
    static uint32x2 _psp_spatial_render_size(const uint32x2 size)
    {
        constexpr float32 _max_width = 256.f;
        constexpr float32 _max_height = 144.f;
        if (size.x == 0 || size.y == 0) {
            return uint32x2(1, 1);
        }
        const float32 _scale = std::min({ 1.f, _max_width / static_cast<float32>(size.x), _max_height / static_cast<float32>(size.y) });
        return uint32x2(
            std::max<uint32>(1, static_cast<uint32>(static_cast<float32>(size.x) * _scale)),
            std::max<uint32>(1, static_cast<uint32>(static_cast<float32>(size.y) * _scale)));
    }
#endif

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
    _viewport_geometry._cached->fetched.on_ready([this](detail::asset_geometry& _on_ready_geometry) {
#if !defined(LUCARIA_BACKEND_PSPGU)
        _invert_texcoords(_on_ready_geometry.data.texcoords);
#endif
        _viewport_mesh.emplace(_viewport_geometry.value());
    });

    _viewport_size = size;
#if defined(LUCARIA_BACKEND_PSPGU)
    _imgui_render_size = _psp_spatial_render_size(size);
#else
    _imgui_render_size = size;
#endif
    _imgui_color_texture.emplace(_imgui_render_size);
    _imgui_framebuffer.emplace();
    _imgui_framebuffer->bind_color(_imgui_color_texture.value().texture);
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
