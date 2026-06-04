#pragma once

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_cache.hpp>

namespace lucaria {

namespace detail {

    template <typename AssetType>
    struct handle_asset_mapping {
        static uint32 save(mappings_manager_object_save& mappings, const assets_cell<AssetType>* cell)
        {
            return mappings.user_assets.template get_or_create<AssetType>(cell);
        }

        static assets_cell<AssetType>* load(mappings_manager_object_load& mappings, const uint32 id)
        {
            return mappings.user_assets.template get<AssetType>(id);
        }

        static constexpr const char* name()
        {
            return "user_asset";
        }
    };

#define LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(AssetType, SaveMember, LoadMember, NameLiteral) \
    template <>                                                                       \
    struct handle_asset_mapping<AssetType> {                                           \
        static uint32 save(mappings_manager_object_save& mappings, const assets_cell<AssetType>* cell) \
        {                                                                              \
            return mappings.SaveMember.get(cell);                                      \
        }                                                                              \
        static assets_cell<AssetType>* load(mappings_manager_object_load& mappings, const uint32 id) \
        {                                                                              \
            return mappings.LoadMember.get(id);                                        \
        }                                                                              \
        static constexpr const char* name()                                            \
        {                                                                              \
            return NameLiteral;                                                        \
        }                                                                              \
    };

    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_animation, animations, animations, "animation")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_audio, audios, audios, "audio")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_cubemap, cubemaps, cubemaps, "cubemap")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_event_track, event_tracks, event_tracks, "event_track")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_font, fonts, fonts, "font")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_geometry, geometries, geometries, "geometry")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_image, images, images, "image")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_mesh, meshes, meshes, "mesh")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_motion_track, motion_tracks, motion_tracks, "motion_track")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_shape, shapes, shapes, "shape")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_skeleton, skeletons, skeletons, "skeleton")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_sound_track, sound_tracks, sound_tracks, "sound_track")
    LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(object_texture, textures, textures, "texture")

#undef LUCARIA_DEFINE_HANDLE_ASSET_MAPPING

}

template <typename AssetType>
struct handle_asset {

    bool has_value() const
    {
        return _cached != nullptr && _cached->fetched.has_value();
    }

    explicit operator bool() const
    {
        return has_value();
    }

    AssetType& value()
    {
        LUCARIA_DEBUG_ASSERT(_cached != nullptr, "Invalid asset handle");
        return _cached->fetched.value();
    }

    const AssetType& value() const
    {
        LUCARIA_DEBUG_ASSERT(_cached != nullptr, "Invalid asset handle");
        return _cached->fetched.value();
    }

    AssetType* operator->()
    {
        return &value();
    }

    const AssetType* operator->() const
    {
        return &value();
    }

    AssetType& operator*()
    {
        return value();
    }

    const AssetType& operator*() const
    {
        return value();
    }

    void reset()
    {
        _cached = nullptr;
        _refcount = {};
    }

    bool empty() const
    {
        return _cached == nullptr;
    }

// private for API purposes, public for existing engine internals during migration.
    detail::flag_refcount _refcount = {};
    detail::assets_cell<AssetType>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        detail::mappings_manager_game_save& mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
        const uint32 asset_id = _cached == nullptr ? 0u : detail::handle_asset_mapping<AssetType>::save(mappings.objects, _cached);
        archive(cereal::make_nvp("object_save_id", asset_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
        uint32 asset_id = 0;
        archive(cereal::make_nvp("object_save_id", asset_id));

        if (asset_id == 0) {
            reset();
            return;
        }

        detail::mappings_manager_game_load& mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);
        detail::assets_cell<AssetType>* cached_cell = detail::handle_asset_mapping<AssetType>::load(mappings.objects, asset_id);

        if (cached_cell == nullptr) {
            LUCARIA_DEBUG_ERROR(std::string("Failed to resolve ") + detail::handle_asset_mapping<AssetType>::name() + " handle while loading");
            reset();
            return;
        }

        _cached = cached_cell;
        _refcount = detail::flag_refcount(&_cached->refcount_control);
    }

    friend struct context_object;
    friend class cereal::access;
};

}
