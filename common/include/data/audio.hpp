#pragma once

#include <vector>

#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

struct audio_data {
    glm::uint channels;
    glm::uint length;
    std::vector<glm::float32> samples;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("channels", channels));
        archive(cereal::make_nvp("length", length));
        archive(cereal::make_nvp("samples", samples));
    }
};