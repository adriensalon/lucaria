#pragma once

#include <imgui.h>
#include <core/window.hpp>

struct immediate_system {
    immediate_system() = delete;
    immediate_system(const immediate_system& other) = delete;
    immediate_system& operator=(const immediate_system& other) = delete;
    immediate_system(immediate_system&& other) = delete;
    immediate_system& operator=(immediate_system&& other) = delete;
    
    static void use_gui_mvp(const std::optional<glm::mat4>& mvp);

    [[nodiscard]] static ImDrawList* get_gui_drawlist();
    [[nodiscard]] static std::unordered_map<keyboard_key, bool>& get_keys();
    [[nodiscard]] static std::unordered_map<glm::uint, bool>& get_buttons();
    [[nodiscard]] static glm::vec2 get_screen_size();
    [[nodiscard]] static glm::vec2 get_mouse_position();
    [[nodiscard]] static glm::vec2& get_mouse_position_delta();
    [[nodiscard]] static glm::float64 get_time_delta();

};