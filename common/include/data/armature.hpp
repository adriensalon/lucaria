#pragma once

#include <vector>

/// @brief Represents an armature for serialization
struct armature_data {
    unsigned int count;
    std::vector<float> weights = {};
    std::vector<unsigned int> bones = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(count);
        archive(weights);
        archive(bones);
    }
};