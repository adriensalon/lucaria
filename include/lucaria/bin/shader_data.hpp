#pragma once

#include <lucaria/bin/container_types.hpp>

namespace lucaria {

enum struct shader_data_type {
	glsl,
};

struct shader_data {
	shader_data_type type = shader_data_type::glsl;
    std::string text = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("text", text));
    }
};

}
