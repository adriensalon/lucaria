#pragma once

#include <filesystem>

namespace lucaria {

/// @brief Represents a shader for serialization
struct shader {
    std::string text;
};

/// @brief Imports a shader as data
/// @param input the shader path to load, containing text
/// @return the shader data as a string
shader import_shader(const std::filesystem::path& input);

/// @brief Compiles a shader to a binary file
/// @param data the shader data to compile as binary
/// @param output the binary path to save the shader to
void compile_shader(const shader& data, const std::filesystem::path& output);

}