#pragma once

#include <filesystem>
#include <vector>

namespace lucaria {

/// @brief Represents a texture for serialization
struct texture {
    unsigned int channels;
    unsigned int width;
    unsigned int height;
    std::vector<unsigned char> pixels;
};

/// @brief Imports a texture as data
/// @param input the texture path to load, containing 3 (RGB) or 4 (RGBA) channels
/// @return the texture data with pixels stored as unsigned char (0-255)
texture import_texture(const std::filesystem::path& input);

/// @brief Compiles a texture to a binary file
/// @param data the texture data to compile as binary
/// @param output the binary path to save the texture to
void compile_texture(const texture& data, const std::filesystem::path& output);

}