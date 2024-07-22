#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <unordered_map>

void graphics_assert();
void audio_assert();

void run(std::function<void()> update);
glm::uint get_samplerate();
std::unordered_map<std::string, bool>& get_keys();
std::unordered_map<glm::uint, bool>& get_buttons();
glm::vec2 get_screen_size();
glm::vec2 get_mouse_position();
glm::vec2& get_mouse_position_delta();
glm::float64 get_time_delta();
bool is_audio_locked();
bool is_mouse_locked();