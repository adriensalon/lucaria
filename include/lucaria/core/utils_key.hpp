#pragma once

#include <string>
#include <unordered_map>

#include <lucaria/public/utils_key.hpp>

#if defined(LUCARIA_PLATFORM_ANDROID)
#include <lucaria/core/platform_android.hpp>
#endif

#if defined(LUCARIA_PLATFORM_WEB)
#include <lucaria/core/platform_web.hpp>
#endif

#if defined(LUCARIA_PLATFORM_GLFW)
#include <lucaria/core/platform_glfw.hpp>
#endif

#if defined(LUCARIA_PLATFORM_PSP)
#include <lucaria/core/platform_psp.hpp>
#endif

namespace lucaria {
namespace detail {

#if defined(LUCARIA_PLATFORM_ANDROID)
    static const std::unordered_map<std::string, input_key> android_keyboard_mappings = {
        { "a", input_key::keyboard_a },
        { "z", input_key::keyboard_z },
        { "e", input_key::keyboard_e },
        { "r", input_key::keyboard_r },
        { "t", input_key::keyboard_t },
        { "y", input_key::keyboard_y },
        { "u", input_key::keyboard_u },
        { "i", input_key::keyboard_i },
        { "o", input_key::keyboard_o },
        { "p", input_key::keyboard_p },
        { "q", input_key::keyboard_q },
        { "s", input_key::keyboard_s },
        { "d", input_key::keyboard_d },
        { "f", input_key::keyboard_f },
        { "g", input_key::keyboard_g },
        { "h", input_key::keyboard_h },
        { "j", input_key::keyboard_j },
        { "k", input_key::keyboard_k },
        { "l", input_key::keyboard_l },
        { "m", input_key::keyboard_m },
        { "w", input_key::keyboard_w },
        { "x", input_key::keyboard_x },
        { "c", input_key::keyboard_c },
        { "v", input_key::keyboard_v },
        { "b", input_key::keyboard_b },
        { "n", input_key::keyboard_n },
    };

#endif

#if defined(LUCARIA_PLATFORM_GLFW)
    static const std::unordered_map<int, input_key> glfw_keyboard_mappings = {
        { GLFW_KEY_A, input_key::keyboard_a },
        { GLFW_KEY_Z, input_key::keyboard_z },
        { GLFW_KEY_E, input_key::keyboard_e },
        { GLFW_KEY_R, input_key::keyboard_r },
        { GLFW_KEY_T, input_key::keyboard_t },
        { GLFW_KEY_Y, input_key::keyboard_y },
        { GLFW_KEY_U, input_key::keyboard_u },
        { GLFW_KEY_I, input_key::keyboard_i },
        { GLFW_KEY_O, input_key::keyboard_o },
        { GLFW_KEY_P, input_key::keyboard_p },
        { GLFW_KEY_Q, input_key::keyboard_q },
        { GLFW_KEY_S, input_key::keyboard_s },
        { GLFW_KEY_D, input_key::keyboard_d },
        { GLFW_KEY_F, input_key::keyboard_f },
        { GLFW_KEY_G, input_key::keyboard_g },
        { GLFW_KEY_H, input_key::keyboard_h },
        { GLFW_KEY_J, input_key::keyboard_j },
        { GLFW_KEY_K, input_key::keyboard_k },
        { GLFW_KEY_L, input_key::keyboard_l },
        { GLFW_KEY_M, input_key::keyboard_m },
        { GLFW_KEY_W, input_key::keyboard_w },
        { GLFW_KEY_X, input_key::keyboard_x },
        { GLFW_KEY_C, input_key::keyboard_c },
        { GLFW_KEY_V, input_key::keyboard_v },
        { GLFW_KEY_B, input_key::keyboard_b },
        { GLFW_KEY_N, input_key::keyboard_n },
    };

#endif

#if defined(LUCARIA_PLATFORM_WEB)
    static const std::unordered_map<std::string, input_key> emscripten_keyboard_mappings = {
        { "a", input_key::keyboard_a },
        { "z", input_key::keyboard_z },
        { "e", input_key::keyboard_e },
        { "r", input_key::keyboard_r },
        { "t", input_key::keyboard_t },
        { "y", input_key::keyboard_y },
        { "u", input_key::keyboard_u },
        { "i", input_key::keyboard_i },
        { "o", input_key::keyboard_o },
        { "p", input_key::keyboard_p },
        { "q", input_key::keyboard_q },
        { "s", input_key::keyboard_s },
        { "d", input_key::keyboard_d },
        { "f", input_key::keyboard_f },
        { "g", input_key::keyboard_g },
        { "h", input_key::keyboard_h },
        { "j", input_key::keyboard_j },
        { "k", input_key::keyboard_k },
        { "l", input_key::keyboard_l },
        { "m", input_key::keyboard_m },
        { "w", input_key::keyboard_w },
        { "x", input_key::keyboard_x },
        { "c", input_key::keyboard_c },
        { "v", input_key::keyboard_v },
        { "b", input_key::keyboard_b },
        { "n", input_key::keyboard_n },
    };
#endif

}
}