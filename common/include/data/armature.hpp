#pragma once

#include <vector>

#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

#include <data/glm.hpp>

struct armature_data {
    glm::uint count;
    std::vector<glm::vec3> positions = {};
    std::vector<glm::vec4> weights = {};
    std::vector<glm::uvec4> bones = {};
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("count", count));
        archive(cereal::make_nvp("positions", positions));
        archive(cereal::make_nvp("weights", weights));
        archive(cereal::make_nvp("bones", bones));
    }
};