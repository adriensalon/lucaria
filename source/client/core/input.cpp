#include <lucaria/core/input.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {

// std::unordered_map<input_key, detail::key_event> _button_events = {};
// std::unordered_map<glm::uint, detail::pointer_event> _pointer_events = {};

namespace detail {

    bool get_is_keyboard_supported()
    {
        return engine_window().is_keyboard_supported;
    }

    bool get_is_mouse_supported()
    {
        return engine_window().is_mouse_supported;
    }

    bool get_is_touch_supported()
    {
        return engine_window().is_touch_supported;
    }

    std::unordered_map<input_key, key_event>& get_buttons()
    {
        return engine_window().key_events;
    }

    std::unordered_map<uint32, pointer_event>& get_pointers()
    {
        return engine_window().pointer_events;
    }

    float32x2 get_mouse_position()
    {
        return engine_window().pointer_events[0].position;
    }

    float32x2& get_mouse_position_delta()
    {
        return engine_window().pointer_events[0].delta;
    }

}

bool input_context::is_keyboard_supported()
{
    return detail::get_is_keyboard_supported();
}

bool input_context::is_mouse_supported()
{
    return detail::get_is_mouse_supported();
}

bool input_context::is_touch_supported()
{
    return detail::get_is_touch_supported();
}

std::unordered_map<input_key, detail::key_event>& input_context::button_events()
{
    return detail::get_buttons();
}

std::unordered_map<uint32, detail::pointer_event>& input_context::pointer_events()
{
    return detail::get_pointers();
}

float32x2 input_context::mouse_position()
{
    return detail::get_mouse_position();
}

float32x2 input_context::mouse_position_delta()
{
    return detail::get_mouse_position_delta();
}

}