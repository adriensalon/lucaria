#pragma once

#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

namespace lucaria {

struct audio_data {
    glm::uint sample_rate;
    std::vector<glm::float32> samples;

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("sample_rate", sample_rate));
        archive(cereal::make_nvp("samples", samples));
    }
};

}
