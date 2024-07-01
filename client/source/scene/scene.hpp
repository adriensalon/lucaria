#pragma once

#include <array>
#include <chrono>
#include <future>

#include <glm/glm.hpp>

#include <core/mesh.hpp>
#include <core/texture.hpp>

/// @brief
/// @param color
/// @param depth
void update_camera(const glm::vec4& color = { 0, 0, 0, 1 }, const bool depth = true);

/// @brief 
void update_controller();

/// @brief
/// @param mesh
/// @param texture
void update_room(std::future<mesh_data>& mesh, std::future<texture_data>& texture);

/// @brief
/// @param textures
void update_skybox(std::array<std::future<texture_data>, 6>& textures);

/// @brief
/// @param duration
void update_splash(const std::chrono::seconds& duration);