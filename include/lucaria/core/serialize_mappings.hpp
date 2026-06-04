#pragma once

#include <lucaria/core/storage_entity.hpp>
#include <lucaria/core/user_asset.hpp>
#include <lucaria/core/object_animation.hpp>
#include <lucaria/core/object_audio.hpp>
#include <lucaria/core/object_cubemap.hpp>
#include <lucaria/core/object_event_track.hpp>
#include <lucaria/core/object_font.hpp>
#include <lucaria/core/object_geometry.hpp>
#include <lucaria/core/object_image.hpp>
#include <lucaria/core/object_mesh.hpp>
#include <lucaria/core/object_motion_track.hpp>
#include <lucaria/core/object_shape.hpp>
#include <lucaria/core/object_skeleton.hpp>
#include <lucaria/core/object_sound_track.hpp>
#include <lucaria/core/object_texture.hpp>

namespace lucaria {

struct context_dynamics;

namespace detail {

    struct manager_assets;
    struct manager_scenes;
    struct object_user_scene;

    struct mappings_user_asset_base_save {
        virtual ~mappings_user_asset_base_save() = default;
    };

    template <typename AssetType>
    struct mappings_user_asset_save final : mappings_user_asset_base_save {
        mappings_container_cache_vector_save<AssetType> typed_mapping = {};
    };

    struct mappings_user_assets_save {

        template <typename AssetType>
        [[nodiscard]] mappings_container_cache_vector_save<AssetType>& get_mapping()
        {
            const std::type_index _type = std::type_index(typeid(AssetType));
            typename std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_save>>::const_iterator _iterator = _mappings.find(_type);
            if (_iterator == _mappings.end()) {
                std::unique_ptr<mappings_user_asset_save<AssetType>> _mapping_ptr = std::make_unique<mappings_user_asset_save<AssetType>>();
                mappings_user_asset_save<AssetType>* _mapping_raw = _mapping_ptr.get();
                _mappings.emplace(_type, std::move(_mapping_ptr));
                return _mapping_raw->typed_mapping;
            }
            return static_cast<mappings_user_asset_save<AssetType>*>(_iterator->second.get())->typed_mapping;
        }

        template <typename AssetType>
        [[nodiscard]] uint32 get_or_create(const assets_cell<AssetType>* cell)
        {
            return get_mapping<AssetType>().get_or_create(cell);
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_save>> _mappings = {};
    };

    struct mappings_user_asset_base_load {
        virtual ~mappings_user_asset_base_load() = default;
    };

    template <typename AssetType>
    struct mappings_user_asset_load final : mappings_user_asset_base_load {
        mappings_container_cache_vector_load<AssetType> typed_mapping = {};
    };

    struct mappings_user_assets_load {

        template <typename AssetType>
        [[nodiscard]] mappings_container_cache_vector_load<AssetType>& get_mapping()
        {
            const std::type_index _type = std::type_index(typeid(AssetType));
            typename std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_load>>::const_iterator _iterator = _mappings.find(_type);
            if (_iterator == _mappings.end()) {
                std::unique_ptr<mappings_user_asset_load<AssetType>> _mapping_ptr = std::make_unique<mappings_user_asset_load<AssetType>>();
                mappings_user_asset_load<AssetType>* _mapping_raw = _mapping_ptr.get();
                _mappings.emplace(_type, std::move(_mapping_ptr));
                return _mapping_raw->typed_mapping;
            }
            return static_cast<mappings_user_asset_load<AssetType>*>(_iterator->second.get())->typed_mapping;
        }

        template <typename AssetType>
        void set(uint32 id, assets_cell<AssetType>* cell)
        {
            get_mapping<AssetType>().set(id, cell);
        }

        template <typename AssetType>
        [[nodiscard]] assets_cell<AssetType>* get(uint32 id)
        {
            return get_mapping<AssetType>().get(id);
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<mappings_user_asset_base_load>> _mappings = {};
    };

    struct mappings_manager_object_save {
        mappings_container_cache_vector_save<object_animation> animations = {};
        mappings_container_cache_vector_save<object_audio> audios = {};
        mappings_container_cache_vector_save<object_cubemap> cubemaps = {};
        mappings_container_cache_vector_save<object_event_track> event_tracks = {};
        mappings_container_cache_vector_save<object_font> fonts = {};
        mappings_container_cache_vector_save<object_geometry> geometries = {};
        mappings_container_cache_vector_save<object_image> images = {};
        mappings_container_cache_vector_save<object_mesh> meshes = {};
        mappings_container_cache_vector_save<object_motion_track> motion_tracks = {};
        mappings_container_cache_vector_save<object_shape> shapes = {};
        mappings_container_cache_vector_save<object_skeleton> skeletons = {};
        mappings_container_cache_vector_save<object_sound_track> sound_tracks = {};
        mappings_container_cache_vector_save<object_texture> textures = {};

        mappings_user_assets_save user_assets = {};
    };

    struct mappings_manager_object_load {
        mappings_container_cache_vector_load<object_image> images = {};
        mappings_container_cache_vector_load<object_texture> textures = {};
        mappings_container_cache_vector_load<object_cubemap> cubemaps = {};
        mappings_container_cache_vector_load<object_geometry> geometries = {};
        mappings_container_cache_vector_load<object_shape> shapes = {};
        mappings_container_cache_vector_load<object_mesh> meshes = {};
        mappings_container_cache_vector_load<object_font> fonts = {};
        mappings_container_cache_vector_load<object_audio> audios = {};
        mappings_container_cache_vector_load<object_sound_track> sound_tracks = {};
        mappings_container_cache_vector_load<object_skeleton> skeletons = {};
        mappings_container_cache_vector_load<object_animation> animations = {};
        mappings_container_cache_vector_load<object_motion_track> motion_tracks = {};
        mappings_container_cache_vector_load<object_event_track> event_tracks = {};

        mappings_user_assets_load user_assets = {};
    };

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
    };

    struct mappings_manager_game_load {
        mappings_manager_object_load objects = {};
        mappings_manager_scene_load scenes = {};
        manager_assets* loading_objects = nullptr;
        context_dynamics* dynamics = nullptr;
        manager_scenes* loading_scene_manager = nullptr;
        object_user_scene* loading_scene = nullptr;
        uint32 loading_scene_save_id = 0;
    };

}
}