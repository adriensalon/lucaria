#pragma once

#include <cereal/types/vector.hpp>

#include <lucaria/common/math_data.hpp>

namespace lucaria {

struct geometry_data {
    glm::uint count = 0; // vertices count
    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec4> colors = {};
    std::vector<glm::vec3> normals = {};
    std::vector<glm::vec3> tangents = {};
    std::vector<glm::vec3> bitangents = {};
    std::vector<glm::vec2> texcoords = {};
    std::vector<glm::ivec4> bones = {};
    std::vector<glm::vec4> weights = {};
    // std::vector<glm::uvec2> line_indices = {};
    std::vector<glm::uvec3> indices = {};
    // std::vector<glm::uvec4> quad_indices = {};
    std::vector<glm::mat4> invposes = {};
    
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
        archive(cereal::make_nvp("bones", bones));
        archive(cereal::make_nvp("weights", weights));
        archive(cereal::make_nvp("indices", indices));
        archive(cereal::make_nvp("invposes", invposes));
    }
};

}
