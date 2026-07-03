#pragma once

#include <AL/al.h>

#include <lucaria/engine/context_serialize.hpp>
#include <lucaria/core/utils_owning.hpp>
#include <lucaria/engine/asset_audio.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

    struct manager_assets;

    enum struct object_sound_track_origin {
        path,
        data
    };

    struct object_sound_track {
        object_sound_track() = default;
        object_sound_track(const object_sound_track& other) = delete;
        object_sound_track& operator=(const object_sound_track& other) = delete;
        object_sound_track(object_sound_track&& other) = default;
        object_sound_track& operator=(object_sound_track&& other) = default;
        ~object_sound_track();

        object_sound_track(const asset_audio& from);

        object_sound_track_origin origin;
        std::filesystem::path origin_path;
        flag_owning ownership = {};
        ALuint id;
        uint32 sample_rate;
        uint32 samples_count;

        void save(context_save_storage& context) const
        {
            context.field("origin", origin);
            if (origin == object_sound_track_origin::path) {
                context.field("origin_path", origin_path);
            }
        }

        void load(context_load_storage& context)
        {
            context.field("origin", origin);
            if (origin == object_sound_track_origin::path) {
                context.field("origin_path", origin_path);
                const std::filesystem::path _path = origin_path;
                const std::filesystem::path _resolved_path = resolve_profile(context.objects, _path, std::optional<data_audio_profile> {});
                context.fetch(_resolved_path, [this, _path](const std::vector<char>& bytes) {
                    asset_audio _audio(bytes);
                    *this = object_sound_track(_audio);
                    origin_path = _path;
                });
            }
        }
    };
}

struct handle_sound_track : handle_asset<detail::object_sound_track> {
    using handle_asset<detail::object_sound_track>::handle_asset;
};

}
