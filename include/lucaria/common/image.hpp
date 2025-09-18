#pragma once

#include <vector>

#include <cereal/types/vector.hpp>
#include <glm/glm.hpp>

namespace lucaria {

struct image_data {
    glm::uint channels;
    glm::uint width;
    glm::uint height;
    std::vector<glm::uint8> pixels;
    bool is_compressed_etc = false;
    bool is_compressed_s3tc = false;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("channels", channels));
        archive(cereal::make_nvp("width", width));
        archive(cereal::make_nvp("height", height));
        archive(cereal::make_nvp("pixels", pixels));
    }
};

}
