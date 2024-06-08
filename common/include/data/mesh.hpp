#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace glm {

template <typename archive_t>
void serialize(archive_t& archive, glm::vec2& vec)
{
    archive(vec.x);
    archive(vec.y);
}

template <typename archive_t>
void serialize(archive_t& archive, glm::vec3& vec)
{
    archive(vec.x);
    archive(vec.y);
    archive(vec.z);
}

}

namespace lucaria {

/// @brief Represents a mesh for serialization
struct mesh_data {
    unsigned int count;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;
};

}