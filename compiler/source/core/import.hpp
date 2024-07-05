#pragma once

#include <filesystem>

#include <data/armature.hpp>
#include <data/mesh.hpp>
#include <data/shader.hpp>
#include <data/texture.hpp>

/// @brief 
/// @param input 
/// @param output_directory 
/// @return 
armature_data import_armature(const std::filesystem::path& input);

/// @brief 
/// @param input 
/// @param output_directory 
/// @return 
mesh_data import_mesh(const std::filesystem::path& input);

/// @brief Imports a shader as data
/// @param input the shader path to load, containing text
/// @return the shader data as a string
shader_data import_shader(const std::filesystem::path& input);

/// @brief Imports a texture as data
/// @param input the texture path to load, containing 3 (RGB) or 4 (RGBA) channels
/// @return the texture data with pixels stored as unsigned char (0-255)
texture_data import_texture(const std::filesystem::path& input);