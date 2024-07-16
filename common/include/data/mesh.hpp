#pragma once

#include <vector>

#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

#include <data/glm.hpp>

struct mesh_data {
    glm::uint count = 0;
    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec4> colors = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::vec3> tangents = {};
    std::vector<glm::vec3> bitangents = {};
    std::vector<glm::vec2> texcoords = {};
    std::vector<glm::uvec3> indices = {};
    
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
        archive(cereal::make_nvp("indices", indices));
    }
};