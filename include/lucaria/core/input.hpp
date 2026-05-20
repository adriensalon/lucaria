#pragma once

#include <unordered_map>

#include <lucaria/core/key.hpp>
#include <lucaria/core/math.hpp>

namespace lucaria {
namespace detail {

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

    [[nodiscard]] bool get_is_keyboard_supported();
    [[nodiscard]] bool get_is_mouse_supported();
    [[nodiscard]] bool get_is_touch_supported();
    [[nodiscard]] std::unordered_map<input_key, key_event>& get_buttons();
    [[nodiscard]] std::unordered_map<glm::uint, pointer_event>& get_pointers();
    [[nodiscard]] float32x2 get_mouse_position();
    [[nodiscard]] float32x2& get_mouse_position_delta();

}

struct input_context {

    /// @brief Gets if the implementation provides keyboard events
    /// @return if the feature is supported
    [[nodiscard]] bool is_keyboard_supported();

    /// @brief Gets if the implementation provides mouse events
    /// @return if the feature is supported
    [[nodiscard]] bool is_mouse_supported();

    /// @brief Gets if the implementation provides touch events
    /// @return if the feature is supported
    [[nodiscard]] bool is_touch_supported();

    /// @brief Gets the state of the tracked keyboard keys
    /// @return state of the keys
    [[nodiscard]] std::unordered_map<input_key, detail::key_event>& button_events();

    /// @brief Gets the state of the tracked mouse buttons
    /// @return state of the mouse buttons
    [[nodiscard]] std::unordered_map<uint32, detail::pointer_event>& pointer_events();

    /// @brief Gets the current mouse position
    /// Syntactic sugar for calling pointer_events()[0].position
    /// @return current mouse position
    [[nodiscard]] float32x2 mouse_position();

    /// @brief Gets the current mouse position delta from previous frame
    /// Syntactic sugar for calling pointer_events()[0].delta
    /// @return current mouse position delta from previous frame
    [[nodiscard]] float32x2 mouse_position_delta();
};

}
