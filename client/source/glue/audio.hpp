#pragma once

#include <filesystem>
#include <glm/glm.hpp>

namespace lucaria {

/// @brief
/// @param path
void load_audio_mp3(const std::filesystem::path& path);

/// @brief
void play_audio();

/// @brief
void stop_audio();

/// @brief 
/// @param left 
/// @param right 
void set_audio_positions(const glm::vec3 left, const glm::vec3 right);

/// @brief 
/// @param left 
/// @param right 
void set_audio_directions(const glm::vec3 left, const glm::vec3 right);

/// @brief 
/// @param left 
/// @param right 
void set_audio_velocities(const glm::vec3 left, const glm::vec3 right);

/// @brief 
/// @param loop 
void set_audio_loop(const bool loop);

/// @brief 
/// @param gain 
/// @param min 
/// @param max 
void set_audio_gain(const float gain, const float min = 0.f, const float max = 0.f);

/// @brief 
/// @param pitch
void set_audio_pitch(const float pitch = 1.f);

}