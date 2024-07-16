#pragma once
#include <glm/glm.hpp>

#include <data/glm.hpp>

struct volume_data {
    glm::vec3 minimum;
    glm::vec3 maximum;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("minimum", minimum));
        archive(cereal::make_nvp("maximum", maximum));
    }
};