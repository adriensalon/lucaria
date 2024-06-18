#pragma once

#include <string>

/// @brief Represents a shader for serialization
struct shader_data {
    std::string text;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(text);
    }
};