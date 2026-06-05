#pragma once

#include <lucaria/bin/data_shader.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct object_shader {
        object_shader() = delete;
        object_shader(const object_shader& other) = delete;
        object_shader& operator=(const object_shader& other) = delete;
        object_shader(object_shader&& other) = default;
        object_shader& operator=(object_shader&& other) = default;

		object_shader(const std::vector<char>& bytes);
		object_shader(data_shader&& data);

        data_shader data;
    };
};

}
