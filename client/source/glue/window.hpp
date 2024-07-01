#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <unordered_map>

/// @brief 
/// @param update 
void run(std::function<void()> update);

/// @brief 
/// @return 
std::unordered_map<std::string, bool>& get_keys();

/// @brief 
/// @return 
std::unordered_map<int, bool>& get_buttons();

/// @brief 
/// @return 
glm::vec2 get_screen_size();

/// @brief 
/// @return 
glm::vec2 get_mouse_position();

/// @brief 
/// @return 
glm::vec2 get_mouse_position_delta();

/// @brief 
/// @return 
float get_time_delta();

/// @brief
/// @return
std::size_t get_samplerate();