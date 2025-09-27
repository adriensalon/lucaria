#pragma once

#include <cereal/types/string.hpp>

namespace lucaria {

struct shader_data {
    std::string text;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("text", text));
    }
};

}
