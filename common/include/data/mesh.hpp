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

/// @brief Represents a mesh for serialization
struct mesh_data {
    unsigned int count;
    std::vector<GLfloat> positions;
    std::vector<GLfloat> colors;
    std::vector<GLfloat> normals;
    std::vector<GLfloat> tangents;
    std::vector<GLfloat> bitangents;
    std::vector<GLfloat> texcoords;
    std::vector<unsigned int> indices;
};