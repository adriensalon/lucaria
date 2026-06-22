#pragma once

#include <ozz/animation/runtime/animation.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {

    struct asset_animation {
        asset_animation() = default;
        asset_animation(const asset_animation& other) = delete;
        asset_animation& operator=(const asset_animation& other) = delete;
        asset_animation(asset_animation&& other) = default;
        asset_animation& operator=(asset_animation&& other) = default;

        asset_animation(const std::vector<char>& bytes);

        std::filesystem::path origin_path;
        ozz::animation::Animation animation;

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);
    };
}

struct handle_animation : handle_asset<detail::asset_animation> {
    using handle_asset<detail::asset_animation>::handle_asset;
};

}
