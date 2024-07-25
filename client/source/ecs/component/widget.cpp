#include <ecs/component/widget.hpp>

widget_component& widget_component::gui(const std::function<void()>& callback)
{
    _callback = callback;
    return *this;
}