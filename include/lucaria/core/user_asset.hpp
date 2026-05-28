#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/core/user_traits.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;

    template <typename SceneType>
    inline constexpr bool is_user_asset_v = is_bytes_compatible_v<SceneType> && is_cereal_compatible_v<SceneType>;

    template <typename SceneType>
    inline constexpr void static_assert_user_asset()
    {
        if constexpr (!is_user_asset_v<SceneType>) {
            static_assert(is_bytes_compatible_v<SceneType>, "User asset type must be compatible with construction from const std::vector<char>&");
            static_assert(is_cereal_compatible_v<SceneType>, "User asset type must be compatible with cereal serialization");
        }
    }

    enum struct object_user_asset_origin {
        path,
        data
    };

    template <typename AssetType>
    struct object_user_asset {

        object_user_asset(const std::vector<char>& bytes)
            : origin(object_user_asset_origin::path)
            , data(AssetType(bytes))
        {
        }

        object_user_asset(AssetType&& data)
            : origin(object_user_asset_origin::data)
            , data(std::move(data))
        {
        }

        object_user_asset_origin origin;
        AssetType data;
    };

    struct storage_user_asset_base {
        virtual ~storage_user_asset_base() = default;
    };

    template <typename AssetType>
    struct storage_user_asset final : storage_user_asset_base {
        container_cache_vector<object_user_asset<AssetType>> assets;
    };

    // implemented in manager_assets.hpp
    template <typename AssetType>
    [[nodiscard]] container_cache<object_user_asset<AssetType>>& fetch(
        manager_assets& objects,
        container_cache_vector<object_user_asset<AssetType>>& cached_vector,
        const std::filesystem::path& path);

    template <typename AssetType>
    struct recipe_object_user_asset_path {
        std::filesystem::path path;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::make_nvp("path", path));
        }
    };

    template <typename AssetType>
    struct recipe_object_user_asset_data {
        AssetType value;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(cereal::make_nvp("value", value));
        }
    };

    template <typename AssetType>
    using recipe_object_user_asset = std::variant<
        recipe_object_user_asset_path<AssetType>,
        recipe_object_user_asset_data<AssetType>>;

}
}
