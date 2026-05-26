#pragma once

#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

template <typename AssetType>
struct handle_user_asset {

    bool has_value() const
    {
        return _cached != nullptr && _cached->fetched.has_value();
    }

    explicit operator bool() const
    {
        return has_value();
    }

private:
    detail::flag_refcount _refcount = {};
    detail::container_cache<detail::object_user_asset<AssetType>>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        detail::mappings_manager_game_save& _mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 _asset_id = _mappings.objects.user_assets.get_or_create<AssetType>(_cached);
        archive(cereal::make_nvp("object_save_id", _asset_id));
    }

    template <typename Archive>
    void load(Archive& archive)
    {
        // uint32 asset_id = 0;
        // archive(cereal::make_nvp("object_save_id", asset_id));

        // auto& mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);

        // cached = mappings.objects.user_assets.get<AssetType>(asset_id);
        // refcount = detail::flag_refcount(&cached->refs);
    }

    friend struct context_object;
    friend class cereal::access;
};

}
