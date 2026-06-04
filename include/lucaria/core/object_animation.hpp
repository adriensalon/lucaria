#pragma once

#include <ozz/animation/runtime/animation.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/context_serialize.hpp>
#include <lucaria/core/utils_compiler.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct manager_assets;

    enum struct object_animation_origin {
        path
    };

    struct object_animation {
        object_animation() = default;
        object_animation(const object_animation& other) = delete;
        object_animation& operator=(const object_animation& other) = delete;
        object_animation(object_animation&& other) = default;
        object_animation& operator=(object_animation&& other) = default;

        object_animation(const std::vector<char>& bytes);

        object_animation_origin origin;
        std::filesystem::path origin_path;
        ozz::animation::Animation animation;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            context.field("origin_path", origin_path);
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            context.field("origin_path", origin_path);
            const std::filesystem::path _path = origin_path;
            context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                *this = object_animation(bytes);
                origin_path = _path;
            });
        }
    };
}
}
