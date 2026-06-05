#pragma once

#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {
    struct manager_input;
}

/// @brief Buttons tracked by the implementation.
/// If a keyboard is detected this is how the engine will index key events.
/// If a mouse is detected this is how the engine will index mouse button events
enum struct input_key {

    /// @brief Left button of a mouse
    mouse_left = 0,
    /// @brief Right button of a mouse
    mouse_right = 1,
    /// @brief Middle button of a mouse
    mouse_middle = 2,

    /// @brief A key of a keyboard
    keyboard_a,
    /// @brief Z key of a keyboard
    keyboard_z,
    /// @brief E key of a keyboard
    keyboard_e,
    /// @brief R key of a keyboard
    keyboard_r,
    /// @brief T key of a keyboard
    keyboard_t,
    /// @brief Y key of a keyboard
    keyboard_y,
    /// @brief U key of a keyboard
    keyboard_u,
    /// @brief I key of a keyboard
    keyboard_i,
    /// @brief O key of a keyboard
    keyboard_o,
    /// @brief P key of a keyboard
    keyboard_p,
    /// @brief Q key of a keyboard
    keyboard_q,
    /// @brief S key of a keyboard
    keyboard_s,
    /// @brief D key of a keyboard
    keyboard_d,
    /// @brief F key of a keyboard
    keyboard_f,
    /// @brief G key of a keyboard
    keyboard_g,
    /// @brief H key of a keyboard
    keyboard_h,
    /// @brief J key of a keyboard
    keyboard_j,
    /// @brief K key of a keyboard
    keyboard_k,
    /// @brief L key of a keyboard
    keyboard_l,
    /// @brief M key of a keyboard
    keyboard_m,
    /// @brief W key of a keyboard
    keyboard_w,
    /// @brief X key of a keyboard
    keyboard_x,
    /// @brief C key of a keyboard
    keyboard_c,
    /// @brief V key of a keyboard
    keyboard_v,
    /// @brief B key of a keyboard
    keyboard_b,
    /// @brief N key of a keyboard
    keyboard_n
};

struct key_event {
    bool state = false;
    bool is_down = false;
    bool is_up = false;
};

struct pointer_event {
    float32x2 position = float32x2(0);
    float32x2 delta = float32x2(0);
    bool is_down = false;
    bool is_up = false;
};

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
