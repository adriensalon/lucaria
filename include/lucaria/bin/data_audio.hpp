#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>

namespace lucaria {

struct data_audio {
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
