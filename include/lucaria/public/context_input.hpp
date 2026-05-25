#pragma once

#include <lucaria/core/manager_input.hpp>

namespace lucaria {

/// @brief 
struct context_input {

    /// @brief Gets if the implementation provides keyboard events
    /// @return if the feature is supported
    bool is_keyboard_supported();

    /// @brief Gets if the implementation provides mouse events
    /// @return if the feature is supported
    bool is_mouse_supported();

    /// @brief Gets if the implementation provides touch events
    /// @return if the feature is supported
    bool is_touch_supported();

    /// @brief Gets the state of the tracked keyboard keys
    /// @return state of the keys
    std::unordered_map<input_key, key_event>& button_events();

    /// @brief Gets the state of the tracked mouse buttons
    /// @return state of the mouse buttons
    std::unordered_map<uint32, pointer_event>& pointer_events();

    /// @brief Gets the current mouse position
    /// Syntactic sugar for calling pointer_events()[0].position
    /// @return current mouse position
    float32x2 mouse_position();

    /// @brief Gets the current mouse position delta from previous frame
    /// Syntactic sugar for calling pointer_events()[0].delta
    /// @return current mouse position delta from previous frame
    float32x2 mouse_position_delta();

private:
	detail::manager_input* _manager;
	friend struct access_context;
};

}
