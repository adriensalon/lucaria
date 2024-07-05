#pragma once

#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

/// @brief Represents a mesh for serialization
struct mesh_data {
    glm::uint count;
    std::vector<glm::float32> positions = {};
    std::vector<glm::float32> colors = {};
    std::vector<glm::float32> normals = {};
    std::vector<glm::float32> tangents = {};
    std::vector<glm::float32> bitangents = {};
    std::vector<glm::float32> texcoords = {};
    std::vector<glm::float32> weights = {};
    std::vector<glm::uint> bones = {};
    std::vector<glm::uint> indices = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("count", count));
        archive(cereal::make_nvp("positions", positions));
        archive(cereal::make_nvp("colors", colors));
        archive(cereal::make_nvp("normals", normals));
        archive(cereal::make_nvp("tangents", tangents));
        archive(cereal::make_nvp("bitangents", bitangents));
        archive(cereal::make_nvp("texcoords", texcoords));
        archive(cereal::make_nvp("weights", weights));
        archive(cereal::make_nvp("bones", bones));
        archive(cereal::make_nvp("indices", indices));
    }
};