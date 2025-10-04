#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>
#include <unordered_map>

namespace lucaria {

enum struct keyboard_key {
    // clang-format off
    a, z, e, r, t, y, u, i, o, p,
    q, s, d, f, g, h, j, k, l, m, 
    w, x, c, v, b, n
    // clang-format on
};

[[nodiscard]] bool get_is_etc_supported();

[[nodiscard]] bool get_is_s3tc_supported();

[[nodiscard]] bool get_is_mouse_locked();

[[nodiscard]] bool get_is_audio_locked();

[[nodiscard]] std::unordered_map<keyboard_key, bool>& get_keys();

[[nodiscard]] std::unordered_map<glm::uint, bool>& get_buttons();

[[nodiscard]] glm::vec2 get_screen_size();

[[nodiscard]] glm::vec2 get_mouse_position();

[[nodiscard]] glm::vec2& get_mouse_position_delta();

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
