#include <lucaria/core/window.hpp>
#include <lucaria/ecs/component/interface.hpp>

namespace lucaria {
namespace ecs {

    interface_component<interface_mode::screen>::interface_component()
    {
        _imgui_context = detail::create_shared_context();
        _is_owning = true;
    }

    interface_component<interface_mode::screen>::interface_component(interface_component&& other)
    {
        *this = std::move(other);
    }

    interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::operator=(interface_component&& other)
    {
        _is_owning = true;
        _imgui_callback = std::move(other._imgui_callback);
        _imgui_context = other._imgui_context;
        other._is_owning = false;
        return *this;
    }

    interface_component<interface_mode::screen>::~interface_component()
    {
        if (_is_owning) {
            ImGui::DestroyContext(_imgui_context);
        }
    }

    interface_component<interface_mode::screen>& interface_component<interface_mode::screen>::set_callback(const std::function<void()>& callback)
    {
        _imgui_callback = callback;
        return *this;
    }

}
}