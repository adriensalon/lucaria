#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>

namespace lucaria {

enum struct data_image_profile {
	binary,
	s3tc_compressed,
	etc2_compressed,
};

struct data_image {
	data_image_profile profile = data_image_profile::binary;
    uint32 channels = 0;
    uint32 width = 0;
    uint32 height = 0;
    std::vector<uint8> pixels = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("profile", profile));
        archive(cereal::make_nvp("channels", channels));
        archive(cereal::make_nvp("width", width));
        archive(cereal::make_nvp("height", height));
        archive(cereal::make_nvp("pixels", pixels));
    }
};

}
