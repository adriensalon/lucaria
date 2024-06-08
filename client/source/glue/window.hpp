#pragma once

#include <functional>
#include <unordered_map>
#include <glm/glm.hpp>

namespace lucaria {

/// @brief Registers an infinite loop to be run by the browser at the best frequency possible.
/// @param update callback that will be triggered once per frame.
void run(std::function<void()> update);

/// @brief 
/// @return 
const std::unordered_map<std::string, bool>& get_keys();

/// @brief 
/// @return 
const std::unordered_map<int, bool>& get_buttons();

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

}