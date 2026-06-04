#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>
#include <lucaria/public/utils_key.hpp>

namespace lucaria {
namespace detail {

    struct manager_input {
        manager_input() = default;
        manager_input(const manager_input& other) = delete;
        manager_input& operator=(const manager_input& other) = delete;
        manager_input(manager_input&& other) = delete;
        manager_input& operator=(manager_input&& other) = delete;

        bool is_keyboard_supported = false;
        bool is_mouse_supported = false;
        bool is_touch_supported = false;
        std::unordered_map<uint32, pointer_event> pointer_events = {};
        std::unordered_map<input_key, key_event> key_events = {};
        float32x2 mouse_position = float32x2(0);
        float32x2 mouse_position_delta = float32x2(0);
    };

    // no snapshot needed for now

}
}
