#include <lucaria/core/database.hpp>

namespace lucaria {
namespace detail {

    namespace {

        template <typename ImplementationType, typename RecipeType>
        void _make_all_recipes_for(
            std::vector<recipe_save_entry<RecipeType>>& recipes,
            const implementation_manager<ImplementationType>& implementations,
            implementation_save_ids<ImplementationType>& ids)
        {
            for (const std::unique_ptr<implementation_container<ImplementationType>>& _cell_ptr : implementations.cells) {
                const implementation_container<ImplementationType>* _cell = _cell_ptr.get();
                recipes.push_back(recipe_save_entry<RecipeType> { ids.get_or_create(_cell), make_recipe(*_cell) });
            }
        }

        template <typename ObjectType, typename ImplementationType, typename RecipeType>
        void _apply_recipes_for(
            std::unordered_map<uint32, ObjectType>& objects,
            implementation_manager<ImplementationType>& implementations,
            std::vector<recipe_save_entry<RecipeType>>&& recipes)
        {
            for (recipe_save_entry<RecipeType>& _entry : recipes) {
                // ObjectType _object = consume_recipe(implementations, std::move(_entry.recipe));
                // objects.emplace(_entry.save_id, std::move(_object));
            }
        }
    }

    recipe_save_database implementation_database::make_all_recipes(implementation_save_database& implementations) const
    {
        recipe_save_database _recipes;
        _make_all_recipes_for(_recipes.images, images, implementations.images);
        _make_all_recipes_for(_recipes.textures, textures, implementations.textures);
        _make_all_recipes_for(_recipes.cubemaps, cubemaps, implementations.cubemaps);
        _make_all_recipes_for(_recipes.geometries, geometries, implementations.geometries);
        _make_all_recipes_for(_recipes.shapes, shapes, implementations.shapes);
        _make_all_recipes_for(_recipes.meshes, meshes, implementations.meshes);
        _make_all_recipes_for(_recipes.fonts, fonts, implementations.fonts);
        _make_all_recipes_for(_recipes.audios, audios, implementations.audios);
        _make_all_recipes_for(_recipes.sound_tracks, sound_tracks, implementations.sound_tracks);
        _make_all_recipes_for(_recipes.skeletons, skeletons, implementations.skeletons);
        _make_all_recipes_for(_recipes.animations, animations, implementations.animations);
        _make_all_recipes_for(_recipes.motion_tracks, motion_tracks, implementations.motion_tracks);
        _make_all_recipes_for(_recipes.event_tracks, event_tracks, implementations.event_tracks);

        return _recipes;
    }

    object_save_database implementation_database::apply_recipes(recipe_save_database&& recipes)
    {
        object_save_database _objects;
        _apply_recipes_for(_objects.images, images, std::move(recipes.images));
        _apply_recipes_for(_objects.textures, textures, std::move(recipes.textures));
        _apply_recipes_for(_objects.cubemaps, cubemaps, std::move(recipes.cubemaps));
        _apply_recipes_for(_objects.geometries, geometries, std::move(recipes.geometries));
        _apply_recipes_for(_objects.shapes, shapes, std::move(recipes.shapes));
        _apply_recipes_for(_objects.meshes, meshes, std::move(recipes.meshes));
        _apply_recipes_for(_objects.fonts, fonts, std::move(recipes.fonts));
        _apply_recipes_for(_objects.audios, audios, std::move(recipes.audios));
        _apply_recipes_for(_objects.sound_tracks, sound_tracks, std::move(recipes.sound_tracks));
        _apply_recipes_for(_objects.skeletons, skeletons, std::move(recipes.skeletons));
        _apply_recipes_for(_objects.animations, animations, std::move(recipes.animations));
        _apply_recipes_for(_objects.motion_tracks, motion_tracks, std::move(recipes.motion_tracks));
        _apply_recipes_for(_objects.event_tracks, event_tracks, std::move(recipes.event_tracks));

        return _objects;
    }

    implementation_database& engine_resources()
    {
        static implementation_database _database = {};
        return _database;
    }

    scene_database& engine_scene_database()
    {
        static scene_database _database = {};
        return _database;
    }

}
}