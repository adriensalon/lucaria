#pragma once

#include <filesystem>
#include <glm/glm.hpp>

namespace lucaria {

/// @brief Loads the compile time specified unlit shader. This must be done before the model can
/// be rendered with the draw function, or even loaded with the load_model_gltf function.
void load_unlit_shader();

/// @brief Loads the model from the specified path.
/// @param path 
void load_model_gltf(const std::filesystem::path& path);

/// @brief Clears the default framebuffer.
/// @param color is the color to clear the color buffer with.
/// @param depth specifies if we need to clear the depth buffer too.
void clear(const glm::vec4 color = { 0, 0, 0, 1 }, const bool depth = true);

/// @brief 
/// @param fov
/// @param near
/// @param far
void set_perspective(const float fov = 60.f, const float near = 0.1f, const float far = 100.f);

/// @brief
/// @param rotation
void rotate_camera(const glm::vec2 rotation);

/// @brief
void draw();

}