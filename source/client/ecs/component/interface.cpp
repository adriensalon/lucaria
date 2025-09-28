#include <lucaria/core/window.hpp>
#include <lucaria/ecs/component/interface.hpp>

namespace lucaria {
namespace ecs {

    // screen

    interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::set_callback(const std::function<void()>& callback)
    {
        _imgui_callback = callback;
        return *this;
    }

    //spatial

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
        // create framebuffer from size etc
        return *this;
    }

    interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::use_viewport(fetched<viewport>& from)
    {
        _viewport.emplace(from, [this]() {
            // create framebuffer from size etc
        });
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

    interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_mouse_position_pixels(const glm::uint x, const glm::uint y)
    {
        // todo
        return *this;
    }

    interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_mouse_position_uv(const glm::float32 x, const glm::float32 y)
    {
        // todo
        return *this;
    }

    interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_mouse_down(const glm::uint button)
    {
        // todo
        return *this;
    }

    interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_mouse_up(const glm::uint button)
    {
        // todo
        return *this;
    }

    interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_mouse_click(const glm::uint button, int release_after_frames)
    {
        // todo
        return *this;
    }

    interface_component<interface_mode::spatial>& interface_component<interface_mode::spatial>::set_scroll(const glm::float32 x, const glm::float32 y)
    {
        // todo
        return *this;
    }

}
}