#pragma once

#include <lucaria/bin/container_types.hpp>
#include <lucaria/bin/math_types.hpp>

namespace lucaria {

enum struct image_data_type {
	binary,
	s3tc_compressed,
	etc2_compressed,
};

struct image_data {
	image_data_type type = image_data_type::binary;
    uint32 channels = 0;
    uint32 width = 0;
    uint32 height = 0;
    std::vector<uint8> pixels = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("type", type));
        archive(cereal::make_nvp("channels", channels));
        archive(cereal::make_nvp("width", width));
        archive(cereal::make_nvp("height", height));
        archive(cereal::make_nvp("pixels", pixels));
    }
};

}
