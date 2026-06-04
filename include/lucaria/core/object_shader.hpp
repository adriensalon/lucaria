#pragma once

#include <lucaria/bin/data_shader.hpp>
#include <lucaria/core/utils_compiler.hpp>

#include <lucaria/core/context_serialize.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct object_shader {
        LUCARIA_DELETE_DEFAULT(object_shader)
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
