#pragma once

#include <ozz/animation/runtime/skeleton.h>

#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct system_motion;
    struct manager_assets;

    enum struct object_skeleton_origin {
        path
    };

    struct object_skeleton {
        object_skeleton() = default;
        object_skeleton(const object_skeleton& other) = delete;
        object_skeleton& operator=(const object_skeleton& other) = delete;
        object_skeleton(object_skeleton&& other) = default;
        object_skeleton& operator=(object_skeleton&& other) = default;

        object_skeleton(const std::vector<char>& bytes);

        object_skeleton_origin origin;
        std::filesystem::path origin_path;
        ozz::animation::Skeleton skeleton;

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            if (origin == object_skeleton_origin::path) {
                context.field("origin_path", origin_path);
            }
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            if (origin == object_skeleton_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                    *this = object_skeleton(bytes);
                    origin_path = _path;
                });
            }
        }
    };
}

struct handle_skeleton : handle_asset<detail::object_skeleton> {
    using handle_asset<detail::object_skeleton>::handle_asset;
};

}
