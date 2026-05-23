#pragma once

#include <lucaria/bin/container_types.hpp>
#include <lucaria/bin/math_types.hpp>

namespace lucaria {

struct audio_data {
    uint32 sample_rate = 0;
    std::vector<float32> samples = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("sample_rate", sample_rate));
        archive(cereal::make_nvp("samples", samples));
    }
};

}
