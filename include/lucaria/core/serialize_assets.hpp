#pragma once

#include <lucaria/core/serialize_containers.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/user_asset.hpp>

namespace lucaria {
namespace detail {

    template <typename RecipeType>
    struct recipe_object_entry {
        uint32 save_id;
        RecipeType recipe;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("object_save_id", save_id));
            archive(cereal::make_nvp("recipe", recipe));
        }
    };

    struct storage_user_asset_group {
        std::string type_id = {};
        manager_assets* objects = nullptr;
        user_asset_type_callbacks* callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));

            if constexpr (std::is_base_of_v<cereal::JSONOutputArchive, ArchiveType>) {
                callbacks->json_save(*objects, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryOutputArchive, ArchiveType>) {
                callbacks->binary_save(*objects, archive);
            }
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("type_id", type_id));
            auto& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            if (mappings.loading_objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_object while loading user asset group");
                return;
            }
            auto it = mappings.loading_objects->user_asset_types.find(type_id);
            if (it == mappings.loading_objects->user_asset_types.end()) {
                LUCARIA_DEBUG_ERROR("Unknown user asset type while loading snapshot");
                return;
            }
            if constexpr (std::is_base_of_v<cereal::JSONInputArchive, ArchiveType>) {
                it->second.json_load(*mappings.loading_objects, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryInputArchive, ArchiveType>) {
                it->second.binary_load(*mappings.loading_objects, archive);
            }
        }
    };


    template <typename AssetType>
    [[nodiscard]] assets_buffer<AssetType>& get_asset_buffer(manager_assets& objects);

    template <typename AssetType>
    [[nodiscard]] mappings_container_cache_vector_save<AssetType>& get_asset_mapping(mappings_manager_object_save& mappings);

    template <typename AssetType>
    [[nodiscard]] mappings_container_cache_vector_load<AssetType>& get_asset_mapping(mappings_manager_object_load& mappings);

    template <typename AssetType>
    struct storage_asset_entry {
        uint32 save_id = 0;
        AssetType* value = nullptr;
        manager_assets* objects = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("object_save_id", save_id));
            if (value == nullptr || objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing asset value or manager_assets while saving asset");
                return;
            }
            storage_save_context context { archive, *objects };
            save_user_asset_value(*value, context);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("object_save_id", save_id));

            mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            if (mappings.loading_objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_assets while loading asset");
                return;
            }

            manager_assets& objects = *mappings.loading_objects;
            assets_buffer<AssetType>& buffer = get_asset_buffer<AssetType>(objects);
            assets_cell<AssetType>* cell = buffer.create_cell(container_async<AssetType>::pending(AssetType {}));
            AssetType& asset = cell->fetched.emplaced_value();

            std::shared_ptr<load_storage_context<AssetType, ArchiveType>> context = objects.template make_load_storage_context<AssetType>(archive, cell, asset);
            load_user_asset_value(asset, *context);
            context->close();

            get_asset_mapping<AssetType>(mappings.objects).set(save_id, cell);
        }
    };

#define LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(Type, Member) \
    template <> inline assets_buffer<Type>& get_asset_buffer<Type>(manager_assets& objects) { return objects.Member; } \
    template <> inline mappings_container_cache_vector_save<Type>& get_asset_mapping<Type>(mappings_manager_object_save& mappings) { return mappings.Member; } \
    template <> inline mappings_container_cache_vector_load<Type>& get_asset_mapping<Type>(mappings_manager_object_load& mappings) { return mappings.Member; }

    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_animation, animations)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_audio, audios)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_cubemap, cubemaps)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_event_track, event_tracks)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_font, fonts)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_geometry, geometries)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_image, images)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_mesh, meshes)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_motion_track, motion_tracks)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_shape, shapes)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_skeleton, skeletons)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_sound_track, sound_tracks)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_texture, textures)

#undef LUCARIA_DETAIL_DEFINE_ASSET_STORAGE

    struct recipe_manager_object {
        std::vector<storage_asset_entry<object_animation>> animations = {};
        std::vector<storage_asset_entry<object_audio>> audios = {};
        std::vector<storage_asset_entry<object_cubemap>> cubemaps = {};
        std::vector<storage_asset_entry<object_event_track>> event_tracks = {};
        std::vector<storage_asset_entry<object_font>> fonts = {};
        std::vector<storage_asset_entry<object_geometry>> geometries = {};
        std::vector<storage_asset_entry<object_image>> images = {};
        std::vector<storage_asset_entry<object_mesh>> meshes = {};
        std::vector<storage_asset_entry<object_motion_track>> motion_tracks = {};
        std::vector<storage_asset_entry<object_shape>> shapes = {};
        std::vector<storage_asset_entry<object_skeleton>> skeletons = {};
        std::vector<storage_asset_entry<object_sound_track>> sound_tracks = {};
        std::vector<storage_asset_entry<object_texture>> textures = {};
        std::vector<storage_user_asset_group> user_assets = {};

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("images", images));
            archive(cereal::make_nvp("textures", textures));
            archive(cereal::make_nvp("cubemaps", cubemaps));
            archive(cereal::make_nvp("geometries", geometries));
            archive(cereal::make_nvp("shapes", shapes));
            archive(cereal::make_nvp("meshes", meshes));
            archive(cereal::make_nvp("fonts", fonts));
            archive(cereal::make_nvp("audios", audios));
            archive(cereal::make_nvp("sound_tracks", sound_tracks));
            archive(cereal::make_nvp("skeletons", skeletons));
            archive(cereal::make_nvp("animations", animations));
            archive(cereal::make_nvp("motion_tracks", motion_tracks));
            archive(cereal::make_nvp("event_tracks", event_tracks));
            archive(cereal::make_nvp("user_assets", user_assets));
        }
    };

    template <typename ObjectType>
    void make_storage_entries_for(std::vector<storage_asset_entry<ObjectType>>& entries, manager_assets& objects, const assets_buffer<ObjectType>& cached_vector, mappings_container_cache_vector_save<ObjectType>& ids)
    {
        cached_vector.for_each_cell([&] (assets_cell<ObjectType>& cell) {
            if (!cell.fetched.has_value()) {
                return;
            }
            entries.push_back(storage_asset_entry<ObjectType> { ids.get_or_create(&cell), &cell.fetched.value(), &objects });
        });
    }

    // Legacy recipe helpers kept for source compatibility with older call sites.
    template <typename ObjectType, typename RecipeType>
    void make_recipes_for(std::vector<recipe_object_entry<RecipeType>>&, const assets_buffer<ObjectType>&, mappings_container_cache_vector_save<ObjectType>&)
    {
    }

    template <typename ObjectType, typename RecipeType>
    void apply_recipes_for(manager_assets&, assets_buffer<ObjectType>&, mappings_container_cache_vector_load<ObjectType>&, std::vector<recipe_object_entry<RecipeType>>&)
    {
    }

    void apply_recipes_for(manager_window& window, manager_assets& objects, assets_buffer<object_font>& cached_vector, mappings_container_cache_vector_load<object_font>& mappings, std::vector<recipe_object_entry<recipe_object_font>>& recipes);

    template <typename AssetType>
    struct storage_user_asset_entry {
        uint32 save_id = 0;
        AssetType* value = nullptr;
        manager_assets* objects = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("object_save_id", save_id));

            if (objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_assets while saving user asset");
                return;
            }

            storage_save_context context { archive, *objects };
            save_user_asset_value(*value, context);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("object_save_id", save_id));

            mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            if (mappings.loading_objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_assets while loading user asset");
                return;
            }

            manager_assets& objects = *mappings.loading_objects;
            auto& storage = objects.template get_user_asset_storage<AssetType>();
            assets_cell<AssetType>* cell = storage.assets.create_cell(container_async<AssetType>::pending(AssetType {}));
            AssetType& asset = cell->fetched.emplaced_value();

            std::shared_ptr<load_storage_context<AssetType, ArchiveType>> context = objects.template make_load_storage_context<AssetType>(archive, cell, asset);
            load_user_asset_value(asset, *context);
            context->close();

            mappings.objects.user_assets.template set<AssetType>(save_id, cell);
        }
    };

    template <typename AssetType, typename ArchiveType>
    void save_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        auto& mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
        const auto& storage = objects.template get_user_asset_storage<AssetType>();
        std::vector<storage_user_asset_entry<AssetType>> entries = {};

        storage.assets.for_each_cell([&](assets_cell<AssetType>& cell) {
            if (!cell.fetched.has_value()) {
                return;
            }
            entries.push_back(storage_user_asset_entry<AssetType> {
                mappings.objects.user_assets.template get_mapping<AssetType>().get_or_create(&cell),
                &cell.fetched.value(),
                &objects
            });
        });

        archive(cereal::make_nvp("assets", entries));
    }

    template <typename AssetType, typename ArchiveType>
    void load_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        std::vector<storage_user_asset_entry<AssetType>> entries = {};
        archive(cereal::make_nvp("assets", entries));
    }

    [[nodiscard]] recipe_manager_object make_recipe(manager_assets& objects, mappings_manager_object_save& mappings);
    void apply_recipe(manager_window& window, manager_assets& objects, mappings_manager_object_load& mappings, recipe_manager_object& recipe);


    struct snapshot_assets {
        manager_assets& objects;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            mappings_manager_game_save& mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
            recipe_manager_object storage = make_recipe(objects, mappings.objects);
            archive(storage);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            recipe_manager_object storage = {};
            archive(storage);

            if (mappings.loading_objects == nullptr || mappings.loading_window == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_assets or manager_window while loading snapshot assets");
                return;
            }

            apply_recipe(*mappings.loading_window, *mappings.loading_objects, mappings.objects, storage);
        }
    };

    // declared in manager_assets.hpp
    template <typename AssetType>
    void manager_assets::register_user_asset(std::string type_id)
    {
        user_asset_type_callbacks callbacks = {};

        callbacks.binary_save = [](manager_assets& objects, cereal::PortableBinaryOutputArchive& archive) {
            save_user_asset_group<AssetType>(objects, archive);
        };

        callbacks.binary_load = [](manager_assets& objects, cereal::PortableBinaryInputArchive& archive) {
            load_user_asset_group<AssetType>(objects, archive);
        };

        callbacks.json_save = [](manager_assets& objects, cereal::JSONOutputArchive& archive) {
            save_user_asset_group<AssetType>(objects, archive);
        };

        callbacks.json_load = [](manager_assets& objects, cereal::JSONInputArchive& archive) {
            load_user_asset_group<AssetType>(objects, archive);
        };

        user_asset_type_ids[std::type_index(typeid(AssetType))] = type_id;
        user_asset_types[std::move(type_id)] = std::move(callbacks);

        get_user_asset_storage<AssetType>();
    }
}
}