#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>
#include <unordered_map>

namespace lucaria {

// clang-format off
/// @brief Keyboard keys tracked by the implementation
enum struct keyboard_key {
    a, z, e, r, t, y, u, i, o, p,
    q, s, d, f, g, h, j, k, l, m, 
    w, x, c, v, b, n
};
// clang-format on

/// @brief Gets if the implementation supports ETC2 texture format
/// @return if the feature is supported
[[nodiscard]] bool get_is_etc2_supported();

/// @brief Gets if the implementation supports S3TC texture format
/// @return if the feature is supported
[[nodiscard]] bool get_is_s3tc_supported();

/// @brief Gets if the implementation has locked the mouse
/// @return if the mouse is locked
[[nodiscard]] bool get_is_mouse_locked();

/// @brief Gets if the implementation has enabled audio playback
/// @return if audio playback is enabled
[[nodiscard]] bool get_is_audio_locked();

/// @brief Gets the state of the tracked keyboard keys
/// @return state of the keys
[[nodiscard]] std::unordered_map<keyboard_key, bool>& get_keys();

/// @brief Gets the state of the tracked mouse buttons
/// @return state of the mouse buttons
[[nodiscard]] std::unordered_map<glm::uint, bool>& get_buttons();

/// @brief Gets the current size of the final framebuffer
/// @return size of the screen
[[nodiscard]] glm::vec2 get_screen_size();

/// @brief Gets the current mouse position
/// @return current mouse position
[[nodiscard]] glm::vec2 get_mouse_position();

/// @brief Gets the current mouse position delta from previous frame
/// @return current mouse position delta from previous frame
[[nodiscard]] glm::vec2& get_mouse_position_delta();

/// @brief Gets the current time delta from previous frame
/// @return current time delta from previous frame
[[nodiscard]] glm::float64 get_time_delta();

namespace detail {

    inline std::unique_ptr<ImFontAtlas> imgui_shared_font_atlas = nullptr;
    inline ImGuiContext* imgui_screen_context = nullptr;
    inline glm::uint imgui_shared_font_texture = 0;

    void run_game(const std::function<void()>& start, const std::function<void()>& update);
    ImGuiContext* create_shared_context();
    void reupload_shared_font_texture_RGBA32();

}
}
