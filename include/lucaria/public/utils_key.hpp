#pragma once

namespace lucaria {

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

}