#include <lucaria/core/input.hpp>

namespace lucaria {

std::unordered_map<button_key, button_event> _button_events = {};
std::unordered_map<glm::uint, pointer_event> _pointer_events = {};

extern bool _is_keyboard_supported;
extern bool _is_mouse_supported;
extern bool _is_touch_supported;

bool get_is_keyboard_supported()
{
    return _is_keyboard_supported;
}

bool get_is_mouse_supported()
{
    return _is_mouse_supported;
}

bool get_is_touch_supported()
{
    return _is_touch_supported;
}

std::unordered_map<button_key, button_event>& get_buttons()
{
    return _button_events;
}

std::unordered_map<glm::uint, pointer_event>& get_pointers()
{
    return _pointer_events;
}

glm::vec2 get_mouse_position()
{
    return _pointer_events[0].position;
}

glm::vec2& get_mouse_position_delta()
{
    return _pointer_events[0].delta;
}

}