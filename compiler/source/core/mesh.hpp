#pragma once

#include <filesystem>
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
struct mesh {
    unsigned int count;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<unsigned int> indices;
};

/// @brief Imports a mesh as data
/// @param input the mesh path to load, containing text
/// @return the mesh data as a string
mesh import_mesh(const std::filesystem::path& input);

/// @brief Compiles a mesh to a binary file
/// @param data the mesh data to compile as binary
/// @param output the binary path to save the mesh to
void compile_mesh(const mesh& data, const std::filesystem::path& output);

}