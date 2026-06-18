#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>

namespace lucaria {

enum struct data_image_profile {
    rgba8888,
    rgba5551,
    rgba4444,
    rgb565,
    s3tc_rgb4,
    s3tc_rgba8,
    etc2_rgb4,
    etc2_rgba8,
    palette_indexed4,
    palette_indexed8,
};

struct data_image {
    data_image_profile profile = data_image_profile::rgba8888;
    uint32 channels = 0;
    uint32 width = 0;
    uint32 height = 0;
    std::vector<uint8> pixels = {};
    std::vector<uint8> palette = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("profile", profile));
        archive(cereal::make_nvp("channels", channels));
        archive(cereal::make_nvp("width", width));
        archive(cereal::make_nvp("height", height));
        archive(cereal::make_nvp("pixels", pixels));
        archive(cereal::make_nvp("palette", palette));
    }
};

}
