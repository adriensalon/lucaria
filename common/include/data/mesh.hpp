#pragma once

#include <vector>

/// @brief Represents a mesh for serialization
struct mesh_data {
    unsigned int count;
    std::vector<float> positions;
    std::vector<float> colors;
    std::vector<float> normals;
    std::vector<float> tangents;
    std::vector<float> bitangents;
    std::vector<float> texcoords;
    std::vector<unsigned int> indices;

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(count);
        archive(positions);
        archive(colors);
        archive(normals);
        archive(tangents);
        archive(bitangents);
        archive(texcoords);
        archive(indices);
    }
};