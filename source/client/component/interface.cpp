#include <lucaria/component/interface.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {

// screen

interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::set_callback(const std::function<void()>& callback)
{
    _imgui_callback = callback;
    return *this;
}

// spatial

interface_component<interface_mode::spatial>::interface_component()
{
    _imgui_context = detail::create_shared_context();
    _is_owning = true;
}

interface_component<interface_mode::spatial>::interface_component(interface_component&& other)
{
    *this = std::move(other);
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::operator=(interface_component&& other)
{
    _is_owning = true;
    _viewport = std::move(other._viewport);
    _imgui_callback = std::move(other._imgui_callback);
    _refresh_mode = other._refresh_mode;
    _imgui_context = other._imgui_context;
    _imgui_color_texture = std::move(other._imgui_color_texture);
    _imgui_framebuffer = std::move(other._imgui_framebuffer);
    other._is_owning = false;
    return *this;
}

interface_component<interface_mode::spatial>::~interface_component()
{
    if (_is_owning) {
        ImGui::DestroyContext(_imgui_context);
    }
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::use_viewport(viewport& from)
{
    _viewport.emplace(from);
    const glm::uvec2 _computed_size = _viewport.value().get_computed_screen_size();
    _imgui_color_texture = std::make_unique<texture>(_computed_size);
    _imgui_framebuffer = std::make_unique<framebuffer>();
    _imgui_framebuffer->bind_color(*(_imgui_color_texture.get()));
    return *this;
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::use_viewport(fetched<viewport>& from)
{
    _viewport.emplace(from, [this]() {
        const glm::uvec2 _computed_size = _viewport.value().get_computed_screen_size();
        _imgui_color_texture = std::make_unique<texture>(_computed_size);
        _imgui_framebuffer = std::make_unique<framebuffer>();
        _imgui_framebuffer->bind_color(*(_imgui_color_texture.get()));
    });
    return *this;
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::use_interaction_texture(texture& from)
{
    _interaction_texture.emplace(from);
    return *this;
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::use_interaction_texture(fetched<texture>& from)
{
    _interaction_texture.emplace(from);
    return *this;
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_callback(const std::function<void()>& callback)
{
    _imgui_callback = callback;
    return *this;
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_refresh(const spatial_refresh_mode mode)
{
    _refresh_mode = mode;
    return *this;
}
interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_interaction(const bool interaction)
{
    _use_interaction = interaction;
    return *this;
}

interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_cursor_size(const glm::vec2& cursor_size)
{
    _cursor_size = cursor_size;
    return *this;
}

}