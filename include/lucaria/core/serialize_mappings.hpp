#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

#include <lucaria/core/scenes_entity.hpp>
#include <lucaria/core/serialize_archives.hpp>
#include <lucaria/core/user_assets.hpp>
#include <lucaria/engine/asset_animation.hpp>
#include <lucaria/engine/asset_audio.hpp>
#include <lucaria/engine/asset_cubemap.hpp>
#include <lucaria/engine/asset_event_track.hpp>
#include <lucaria/engine/asset_font.hpp>
#include <lucaria/engine/asset_geometry.hpp>
#include <lucaria/engine/asset_image.hpp>
#include <lucaria/engine/asset_mesh.hpp>
#include <lucaria/engine/asset_motion_track.hpp>
#include <lucaria/engine/asset_shape.hpp>
#include <lucaria/engine/asset_skeleton.hpp>
#include <lucaria/engine/asset_sound_track.hpp>
#include <lucaria/engine/asset_texture.hpp>
#include <lucaria/engine/handle_asset.hpp>
#include <lucaria/engine/handle_entity.hpp>

namespace lucaria {

struct context_dynamics;

namespace detail {

    struct manager_assets;
    struct manager_scenes;
    struct manager_window;
    struct object_user_scene;

    // assets

    template <typename ObjectType>
    struct mappings_container_cache_vector_save {

        [[nodiscard]] uint32 get(const assets_cell<ObjectType>* resource) const
        {
            LUCARIA_DEBUG_ASSERT(resource, "Object implementation was nullptr");
            typename std::unordered_map<const assets_cell<ObjectType>*, uint32>::const_iterator _iterator = _asset_ids.find(resource);
            LUCARIA_DEBUG_ASSERT(_iterator != _asset_ids.end(), "Object was not registered before component snapshot save");
            return _iterator->second;
        }

        [[nodiscard]] uint32 get_or_create(const assets_cell<ObjectType>* resource)
        {
            LUCARIA_DEBUG_ASSERT(resource, "Object implementation was nullptr");
            typename std::unordered_map<const assets_cell<ObjectType>*, uint32>::const_iterator _iterator = _asset_ids.find(resource);
            if (_iterator != _asset_ids.end()) {
                return _iterator->second;
            }
            const uint32 _asset_id = _next_id++;
            _asset_ids.emplace(resource, _asset_id);
            return _asset_id;
        }

    private:
        std::unordered_map<const assets_cell<ObjectType>*, uint32> _asset_ids = {};
        uint32 _next_id = 1;
    };

    template <typename ObjectType>
    struct mappings_container_cache_vector_load {

        void set(const uint32 id, assets_cell<ObjectType>* asset)
        {
            LUCARIA_DEBUG_ASSERT(id != 0 && asset != nullptr, "Invalid object load mapping");
            _assets[id] = asset;
        }

        [[nodiscard]] assets_cell<ObjectType>* get(const uint32 id) const
        {
            typename std::unordered_map<uint32, assets_cell<ObjectType>*>::const_iterator _iterator = _assets.find(id);
            LUCARIA_DEBUG_ASSERT(_iterator != _assets.end(), "Object save id was not loaded before handle load");
            return _iterator->second;
        }

    private:
        std::unordered_map<uint32, assets_cell<ObjectType>*> _assets = {};
    };

    struct mappings_user_asset_base_save {
        virtual ~mappings_user_asset_base_save() = default;
    };

    template <typename Asset>
    struct mappings_user_asset_save final : mappings_user_asset_base_save {
        mappings_container_cache_vector_save<Asset> typed_mapping = {};
    };

    struct mappings_user_assets_save {

        template <typename Asset>
        [[nodiscard]] mappings_container_cache_vector_save<Asset>& get_mapping()
        {
            const std::type_index _type = std::type_index(typeid(Asset));
            typename std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_save>>::const_iterator _iterator = _mappings.find(_type);
            if (_iterator == _mappings.end()) {
                std::unique_ptr<mappings_user_asset_save<Asset>> _mapping_ptr = std::make_unique<mappings_user_asset_save<Asset>>();
                mappings_user_asset_save<Asset>* _mapping_raw = _mapping_ptr.get();
                _mappings.emplace(_type, std::move(_mapping_ptr));
                return _mapping_raw->typed_mapping;
            }
            return static_cast<mappings_user_asset_save<Asset>*>(_iterator->second.get())->typed_mapping;
        }

        template <typename Asset>
        [[nodiscard]] uint32 get_or_create(const assets_cell<Asset>* cell)
        {
            return get_mapping<Asset>().get_or_create(cell);
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_save>> _mappings = {};
    };

    struct mappings_user_asset_base_load {
        virtual ~mappings_user_asset_base_load() = default;
    };

    template <typename Asset>
    struct mappings_user_asset_load final : mappings_user_asset_base_load {
        mappings_container_cache_vector_load<Asset> typed_mapping = {};
    };

    struct mappings_user_assets_load {

        template <typename Asset>
        [[nodiscard]] mappings_container_cache_vector_load<Asset>& get_mapping()
        {
            const std::type_index _type = std::type_index(typeid(Asset));
            typename std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_load>>::const_iterator _iterator = _mappings.find(_type);
            if (_iterator == _mappings.end()) {
                std::unique_ptr<mappings_user_asset_load<Asset>> _mapping_ptr = std::make_unique<mappings_user_asset_load<Asset>>();
                mappings_user_asset_load<Asset>* _mapping_raw = _mapping_ptr.get();
                _mappings.emplace(_type, std::move(_mapping_ptr));
                return _mapping_raw->typed_mapping;
            }
            return static_cast<mappings_user_asset_load<Asset>*>(_iterator->second.get())->typed_mapping;
        }

        template <typename Asset>
        void set(uint32 id, assets_cell<Asset>* cell)
        {
            get_mapping<Asset>().set(id, cell);
        }

        template <typename Asset>
        [[nodiscard]] assets_cell<Asset>* get(uint32 id)
        {
            return get_mapping<Asset>().get(id);
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_load>> _mappings = {};
    };

    struct mappings_manager_object_save {
        mappings_container_cache_vector_save<asset_animation> animations = {};
        mappings_container_cache_vector_save<asset_audio> audios = {};
        mappings_container_cache_vector_save<asset_cubemap> cubemaps = {};
        mappings_container_cache_vector_save<object_event_track> event_tracks = {};
        mappings_container_cache_vector_save<object_font> fonts = {};
        mappings_container_cache_vector_save<asset_geometry> geometries = {};
        mappings_container_cache_vector_save<asset_image> images = {};
        mappings_container_cache_vector_save<asset_mesh> meshes = {};
        mappings_container_cache_vector_save<object_motion_track> motion_tracks = {};
        mappings_container_cache_vector_save<object_shape> shapes = {};
        mappings_container_cache_vector_save<object_skeleton> skeletons = {};
        mappings_container_cache_vector_save<object_sound_track> sound_tracks = {};
        mappings_container_cache_vector_save<asset_texture> textures = {};
        mappings_user_assets_save user_assets = {};
    };

    struct mappings_manager_object_load {
        mappings_container_cache_vector_load<asset_image> images = {};
        mappings_container_cache_vector_load<asset_texture> textures = {};
        mappings_container_cache_vector_load<asset_cubemap> cubemaps = {};
        mappings_container_cache_vector_load<asset_geometry> geometries = {};
        mappings_container_cache_vector_load<object_shape> shapes = {};
        mappings_container_cache_vector_load<asset_mesh> meshes = {};
        mappings_container_cache_vector_load<object_font> fonts = {};
        mappings_container_cache_vector_load<asset_audio> audios = {};
        mappings_container_cache_vector_load<object_sound_track> sound_tracks = {};
        mappings_container_cache_vector_load<object_skeleton> skeletons = {};
        mappings_container_cache_vector_load<asset_animation> animations = {};
        mappings_container_cache_vector_load<object_motion_track> motion_tracks = {};
        mappings_container_cache_vector_load<object_event_track> event_tracks = {};
        mappings_user_assets_load user_assets = {};
    };

    template <typename Asset>
    struct handle_asset_mapping {
		
        static uint32 save(mappings_manager_object_save& mappings, const assets_cell<Asset>* cell)
        {
            return mappings.user_assets.template get_or_create<Asset>(cell);
        }

        static assets_cell<Asset>* load(mappings_manager_object_load& mappings, const uint32 id)
        {
            return mappings.user_assets.template get<Asset>(id);
        }

        static constexpr const char* name()
        {
            return "user_asset";
        }
    };

    // scenes

    struct mappings_manager_scene_save {
        manager_scenes* saving_scene_manager = nullptr;
        std::unordered_map<object_entity_scene_index, uint32> save_map_scene_ids = {};
        std::unordered_map<object_entity_scene_index, std::unordered_map<object_entity, uint32>> save_map_scene_entities = {};
    };

    struct mappings_manager_scene_load {
        std::unordered_map<uint32, object_entity_scene_index> load_map_scenes = {};
        std::unordered_map<uint32, std::unordered_map<uint32, object_entity>> load_map_scene_entities = {};
    };

    struct mappings_manager_game_save {
        mappings_manager_object_save objects = {};
        mappings_manager_scene_save scenes = {};
        manager_assets* saving_objects = nullptr;
    };

    // game

    struct mappings_manager_game_load {
        mappings_manager_object_load objects = {};
        mappings_manager_scene_load scenes = {};
        manager_assets* loading_objects = nullptr;
        manager_window* loading_window = nullptr;
        context_dynamics* dynamics = nullptr;
        manager_scenes* loading_scene_manager = nullptr;
        object_user_scene* loading_scene = nullptr;
        uint32 loading_scene_save_id = 0;
    };

}

template <typename Asset>
template <typename Archive>
void handle_asset<Asset>::save(Archive& archive) const
{
    detail::mappings_manager_game_save& mappings = cereal::get_user_data<detail::mappings_manager_game_save>(archive);
    const uint32 asset_id = _cached == nullptr ? 0u : detail::handle_asset_mapping<Asset>::save(mappings.objects, _cached);
    archive(cereal::make_nvp("object_save_id", asset_id));
}

template <typename Asset>
template <typename Archive>
void handle_asset<Asset>::load(Archive& archive)
{
    uint32 asset_id = 0;
    archive(cereal::make_nvp("object_save_id", asset_id));
    if (asset_id == 0) {
        reset();
        return;
    }
    detail::mappings_manager_game_load& mappings = cereal::get_user_data<detail::mappings_manager_game_load>(archive);
    detail::assets_cell<Asset>* cached_cell = detail::handle_asset_mapping<Asset>::load(mappings.objects, asset_id);
    if (cached_cell == nullptr) {
        LUCARIA_DEBUG_ERROR(std::string("Failed to resolve ") + detail::handle_asset_mapping<Asset>::name() + " handle while loading");
        reset();
        return;
    }
    _cached = cached_cell;
    _refcount = detail::flag_refcount(&_cached->refcount_control);
}

}