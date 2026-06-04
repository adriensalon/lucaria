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

    struct storage_user_asset_base {
        virtual ~storage_user_asset_base() = default;
    };

    template <typename AssetType>
    struct storage_user_asset final : storage_user_asset_base {
        assets_buffer<AssetType> assets;
    };

    // implemented in manager_assets.hpp
    template <typename AssetType>
    [[nodiscard]] assets_cell<AssetType>& fetch(
        manager_assets& objects,
        assets_buffer<AssetType>& cached_vector,
        const std::filesystem::path& path);

}
}
