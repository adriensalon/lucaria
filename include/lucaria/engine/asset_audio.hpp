#pragma once

#include <lucaria/bin/data_audio.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

    enum struct object_audio_origin {
        path,
        data
    };

    struct asset_audio {
        asset_audio() = default;
        asset_audio(const asset_audio& other) = delete;
        asset_audio& operator=(const asset_audio& other) = delete;
        asset_audio(asset_audio&& other) = default;
        asset_audio& operator=(asset_audio&& other) = default;

        asset_audio(const std::vector<char>& bytes);
        asset_audio(data_audio&& data);

        object_audio_origin origin;
        std::filesystem::path origin_path;
        data_audio data;

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);
    };
}

struct handle_audio : handle_asset<detail::asset_audio> {
    using handle_asset<detail::asset_audio>::handle_asset;
};

}
