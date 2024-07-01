#pragma once

#include <vector>

/// @brief Represents a texture for serialization
struct texture_data {
    unsigned int channels;
    unsigned int width;
    unsigned int height;
    std::vector<unsigned char> pixels;
    bool is_compressed;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(channels);
        archive(width);
        archive(height);
        archive(pixels);
        // archive(is_compressed);
    }
};