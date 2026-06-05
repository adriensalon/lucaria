#pragma once

#include <lucaria/bin/data_audio.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

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

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);
    };
}

struct handle_audio : handle_asset<detail::asset_audio> {
    using handle_asset<detail::asset_audio>::handle_asset;
};

}
