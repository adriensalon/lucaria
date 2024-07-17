#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <unordered_map>

void run(std::function<void()> update);
std::unordered_map<std::string, bool>& get_keys();
std::unordered_map<int, bool>& get_buttons();
glm::vec2 get_screen_size();
glm::vec2 get_mouse_position();
glm::vec2 get_mouse_position_delta();
float get_time_delta();
bool is_audio_locked();
bool is_mouse_locked();