#pragma once

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_scenes.hpp>
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
    detail::assets_cell<detail::object_user_asset<AssetType>>* _cached = nullptr;

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
        uint32 asset_id = 0;
        archive(cereal::make_nvp("object_save_id", asset_id));

        if (asset_id == 0) {
            _cached = nullptr;
            _refcount = {};
            return;
        }

        detail::mappings_manager_game_load& mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);

        _cached = mappings.objects.user_assets.template get<AssetType>(asset_id);

        if (_cached == nullptr) {
            LUCARIA_DEBUG_ERROR("Failed to resolve user asset handle while loading");
            _refcount = {};
            return;
        }

        _refcount = detail::flag_refcount(&_cached->refs);
    }

    friend struct context_object;
    friend class cereal::access;
};

}
