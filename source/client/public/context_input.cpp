#include <lucaria/public/context_input.hpp>

namespace lucaria {

bool context_input::is_keyboard_supported()
{
    return _manager->is_keyboard_supported;
}

bool context_input::is_mouse_supported()
{
    return _manager->is_mouse_supported;
}

bool context_input::is_touch_supported()
{
    return _manager->is_touch_supported;
}

std::unordered_map<input_key, key_event>& context_input::button_events()
{
    return _manager->key_events;
}

std::unordered_map<uint32, pointer_event>& context_input::pointer_events()
{
    return _manager->pointer_events;
}

float32x2 context_input::mouse_position()
{
    return _manager->mouse_position;
}

float32x2 context_input::mouse_position_delta()
{
    return _manager->mouse_position_delta;
}

}