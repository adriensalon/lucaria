#pragma once

#include <lucaria/bin/types_containers.hpp>

namespace lucaria {

enum struct data_shader_profile {
	glsl,
};

struct data_shader {
	data_shader_profile type = data_shader_profile::glsl;
    std::string text = {};

    template <typename archive_t>
    void serialize(archive_t& archive)
    {
        archive(cereal::make_nvp("text", text));
    }
};

}
