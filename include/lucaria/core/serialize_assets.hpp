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

    struct recipe_object_user_asset_group {
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

    struct recipe_manager_object {
        std::vector<recipe_object_entry<recipe_object_animation>> animations = {};
        std::vector<recipe_object_entry<recipe_object_audio>> audios = {};
        std::vector<recipe_object_entry<recipe_object_cubemap>> cubemaps = {};
        std::vector<recipe_object_entry<recipe_object_event_track>> event_tracks = {};
        std::vector<recipe_object_entry<recipe_object_font>> fonts = {};
        std::vector<recipe_object_entry<recipe_object_geometry>> geometries = {};
        std::vector<recipe_object_entry<recipe_object_image>> images = {};
        std::vector<recipe_object_entry<recipe_object_mesh>> meshes = {};
        std::vector<recipe_object_entry<recipe_object_motion_track>> motion_tracks = {};
        std::vector<recipe_object_entry<recipe_object_shape>> shapes = {};
        std::vector<recipe_object_entry<recipe_object_skeleton>> skeletons = {};
        std::vector<recipe_object_entry<recipe_object_sound_track>> sound_tracks = {};
        std::vector<recipe_object_entry<recipe_object_texture>> textures = {};
        std::vector<recipe_object_user_asset_group> user_assets = {};

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

    template <typename ObjectType, typename RecipeType>
    void make_recipes_for(std::vector<recipe_object_entry<RecipeType>>& recipes, const assets_buffer<ObjectType>& cached_vector, mappings_container_cache_vector_save<ObjectType>& ids)
    {
		cached_vector.for_each_cell([&] (assets_cell<ObjectType>& cell) {
            recipes.push_back(recipe_object_entry<RecipeType> { ids.get_or_create(&cell), make_recipe(cell) });
		});
    }

    template <typename ObjectType, typename RecipeType>
    void apply_recipes_for(manager_assets& objects, assets_buffer<ObjectType>& cached_vector, mappings_container_cache_vector_load<ObjectType>& mappings, std::vector<recipe_object_entry<RecipeType>>& recipes)
    {
        for (auto& entry : recipes) {
            assets_cell<ObjectType>* cell = apply_recipe(objects, cached_vector, entry.recipe);
            if (cell == nullptr) {
                LUCARIA_DEBUG_ERROR("Failed to apply object recipe");
                continue;
            }
            mappings.set(entry.save_id, cell);
        }
    }

    void apply_recipes_for(manager_window& window, manager_assets& objects, assets_buffer<object_font>& cached_vector, mappings_container_cache_vector_load<object_font>& mappings, std::vector<recipe_object_entry<recipe_object_font>>& recipes);

    template <typename AssetType, typename ArchiveType>
    void save_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        auto& mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
        const auto& storage = objects.template get_user_asset_storage<AssetType>();
        std::vector<recipe_object_entry<recipe_object_user_asset<AssetType>>> recipes = {};
        make_recipes_for(recipes, storage.assets, mappings.objects.user_assets.template get_mapping<AssetType>());
        archive(cereal::make_nvp("assets", recipes));
    }

    template <typename AssetType, typename ArchiveType>
    void load_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
        auto& storage = objects.template get_user_asset_storage<AssetType>();
        std::vector<recipe_object_entry<recipe_object_user_asset<AssetType>>> recipes = {};
        archive(cereal::make_nvp("assets", recipes));
        for (auto& entry : recipes) {
            assets_cell<object_user_asset<AssetType>>* cell = apply_recipe(objects, storage.assets, entry.recipe);
            if (cell == nullptr) {
                LUCARIA_DEBUG_ERROR("Failed to apply user asset recipe");
                continue;
            }
            mappings.objects.user_assets.template set<AssetType>(entry.save_id, cell);
        }
    }

    template <typename AssetType>
    [[nodiscard]] recipe_object_user_asset<AssetType> make_recipe(const assets_cell<object_user_asset<AssetType>>& cache)
    {
        const object_user_asset<AssetType>& _asset = cache.fetched.value();

        if (_asset.origin == object_user_asset_origin::path) {
            return recipe_object_user_asset_path<AssetType> { cache.origin_path.value() };

        } else if (_asset.origin == object_user_asset_origin::data) {
            return recipe_object_user_asset_data<AssetType> { _asset.data };

        } else {
            LUCARIA_DEBUG_ERROR("Implementation error");
            return {};
        }
    }

    template <typename AssetType>
    assets_cell<object_user_asset<AssetType>>* apply_recipe(
        manager_assets& objects,
        assets_buffer<object_user_asset<AssetType>>& cached_vector,
        recipe_object_user_asset<AssetType>& recipe)
    {
        return std::visit([&](auto& value) -> assets_cell<object_user_asset<AssetType>>* {
            using RecipeType = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<RecipeType, recipe_object_user_asset_path<AssetType>>) {
                return &fetch(objects, cached_vector, value.path);

            } else if constexpr (std::is_same_v<RecipeType, recipe_object_user_asset_data<AssetType>>) {
                return cached_vector.create_cell(
                    container_async<object_user_asset<AssetType>>(
                        object_user_asset<AssetType>(std::move(value.value))));

            } else {
                LUCARIA_DEBUG_ERROR("Unhandled user asset recipe type");
                return nullptr;
            }
        },
            recipe);
    }

    [[nodiscard]] recipe_manager_object make_recipe(manager_assets& objects, mappings_manager_object_save& mappings);
    void apply_recipe(manager_window& window, manager_assets& objects, mappings_manager_object_load& mappings, recipe_manager_object& recipe);

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