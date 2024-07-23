#pragma once

#include <vector>

#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

struct audio_data {
    std::vector<glm::float32> samples;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("samples", samples));
    }
};