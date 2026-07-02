#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>

namespace lucaria {

enum struct data_audio_profile {
    float32,
    ogg_vorbis,
};

struct data_audio {
    data_audio_profile profile = data_audio_profile::float32;
    uint32 sample_rate = 0;
    uint32 channels = 0;
    uint32 count = 0;
    std::vector<uint8> samples = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("profile", profile));
        archive(cereal::make_nvp("sample_rate", sample_rate));
        archive(cereal::make_nvp("channels", channels));
        archive(cereal::make_nvp("count", count));
        archive(cereal::make_nvp("samples", samples));
    }
};

}
