#pragma once

#include <string>

#include <cereal/types/string.hpp>

struct shader_data {
    std::string text;
    
    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("text", text));
    }
};