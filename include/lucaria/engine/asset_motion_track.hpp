#pragma once

#include <ozz/animation/runtime/track.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/engine/context_serialize.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

    struct system_motion;
    struct manager_assets;

    enum struct object_motion_track_origin {
        path
    };

    struct object_motion_track {
        object_motion_track() = default;
        object_motion_track(const object_motion_track& other) = delete;
        object_motion_track& operator=(const object_motion_track& other) = delete;
        object_motion_track(object_motion_track&& other) = default;
        object_motion_track& operator=(object_motion_track&& other) = default;

        object_motion_track(const std::vector<char>& bytes);
        [[nodiscard]] float32x3 get_total_translation() const;

        object_motion_track_origin origin;
        std::filesystem::path origin_path;
        ozz::animation::Float3Track translation_track;
        ozz::animation::QuaternionTrack rotation_track;

        void save(context_save_storage& context) const
        {
            context.field("origin", origin);
            if (origin == object_motion_track_origin::path) {
                context.field("origin_path", origin_path);
            }
        }

        void load(context_load_storage& context)
        {
            context.field("origin", origin);
            if (origin == object_motion_track_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                context.fetch(_path, [this, _path](const std::vector<char>& bytes) {
                    *this = object_motion_track(bytes);
                    origin_path = _path;
                });
            }
        }
    };
}

struct handle_motion_track : handle_asset<detail::object_motion_track> {
    using handle_asset<detail::object_motion_track>::handle_asset;
};

}
