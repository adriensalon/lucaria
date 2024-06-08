#pragma once

#include <vector>

/// @brief Represents a texture for serialization
struct texture_data {
    unsigned int channels;
    unsigned int width;
    unsigned int height;
    std::vector<unsigned char> pixels;
};