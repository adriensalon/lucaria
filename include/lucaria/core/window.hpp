#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include <lucaria/core/world.hpp>

namespace lucaria {

enum struct keyboard_key {
    // clang-format off
    a, z, e, r, t, y, u, i, o, p,
    q, s, d, f, g, h, j, k, l, m, 
    w, x, c, v, b, n
    // clang-format on
};

void graphics_assert();
void audio_assert();
void on_audio_locked(const std::function<void()>& callback);
glm::uint get_samplerate();
bool get_is_etc_supported();
bool get_is_s3tc_supported();
bool get_is_mouse_locked();
bool get_is_audio_locked();

ImDrawList* get_gui_drawlist();
void gui_mvp(const std::optional<glm::mat4>& mvp);
std::unordered_map<keyboard_key, bool>& get_keys();
std::unordered_map<glm::uint, bool>& get_buttons();
glm::vec2 get_screen_size();
glm::vec2 get_mouse_position();
glm::vec2& get_mouse_position_delta();
glm::float64 get_time_delta();

void use_imgui_rendering(const bool use);

namespace detail {

    inline std::unique_ptr<ImFontAtlas> imgui_shared_font_atlas = nullptr;
    inline ImGuiContext* imgui_screen_context = nullptr;
    inline glm::uint imgui_shared_font_texture = 0;

    
    void run_impl(const std::function<void()>& start, const std::function<void()>& update);

    ImGuiContext* create_shared_context();
    void reupload_shared_font_texture_RGBA32();

}
}
