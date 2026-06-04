#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/user_traits.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;

    template <typename AssetType>
    inline constexpr bool is_user_asset_v = std::is_default_constructible_v<AssetType>;

    template <typename AssetType>
    inline constexpr void static_assert_user_asset()
    {
        if constexpr (!is_user_asset_v<AssetType>) {
            static_assert(std::is_default_constructible_v<AssetType>, "User asset type must be default constructible");
        }
    }

}
}
