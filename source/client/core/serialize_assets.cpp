#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/serialize_assets.hpp>

namespace lucaria {
namespace detail {

    recipe_manager_object make_recipe(manager_assets& objects, mappings_manager_object_save& mappings)
    {
        recipe_manager_object _recipes;

        make_recipes_for(_recipes.images, objects.images, mappings.images);
        make_recipes_for(_recipes.textures, objects.textures, mappings.textures);
        make_recipes_for(_recipes.cubemaps, objects.cubemaps, mappings.cubemaps);
        make_recipes_for(_recipes.geometries, objects.geometries, mappings.geometries);
        make_recipes_for(_recipes.shapes, objects.shapes, mappings.shapes);
        make_recipes_for(_recipes.meshes, objects.meshes, mappings.meshes);
        make_recipes_for(_recipes.fonts, objects.fonts, mappings.fonts);
        make_recipes_for(_recipes.audios, objects.audios, mappings.audios);
        make_recipes_for(_recipes.sound_tracks, objects.sound_tracks, mappings.sound_tracks);
        make_recipes_for(_recipes.skeletons, objects.skeletons, mappings.skeletons);
        make_recipes_for(_recipes.animations, objects.animations, mappings.animations);
        make_recipes_for(_recipes.motion_tracks, objects.motion_tracks, mappings.motion_tracks);
        make_recipes_for(_recipes.event_tracks, objects.event_tracks, mappings.event_tracks);

        for (const auto& [_type_id, _callbacks] : objects.user_asset_types) {
            storage_user_asset_group& _group = _recipes.user_assets.emplace_back();
            _group.type_id = _type_id;
            _group.objects = const_cast<manager_assets*>(&objects);
            _group.callbacks = const_cast<user_asset_type_callbacks*>(&_callbacks);
        }

        return _recipes;
    }

    void apply_recipe(manager_window& window, manager_assets& objects, mappings_manager_object_load& mappings, recipe_manager_object& recipe)
    {
        apply_recipes_for(objects, objects.images, mappings.images, recipe.images);
        apply_recipes_for(objects, objects.textures, mappings.textures, recipe.textures);
        apply_recipes_for(objects, objects.cubemaps, mappings.cubemaps, recipe.cubemaps);
        apply_recipes_for(objects, objects.geometries, mappings.geometries, recipe.geometries);
        apply_recipes_for(objects, objects.shapes, mappings.shapes, recipe.shapes);
        apply_recipes_for(objects, objects.meshes, mappings.meshes, recipe.meshes);
        apply_recipes_for(window, objects, objects.fonts, mappings.fonts, recipe.fonts);
        apply_recipes_for(objects, objects.audios, mappings.audios, recipe.audios);
        apply_recipes_for(objects, objects.sound_tracks, mappings.sound_tracks, recipe.sound_tracks);
        apply_recipes_for(objects, objects.skeletons, mappings.skeletons, recipe.skeletons);
        apply_recipes_for(objects, objects.animations, mappings.animations, recipe.animations);
        apply_recipes_for(objects, objects.motion_tracks, mappings.motion_tracks, recipe.motion_tracks);
        apply_recipes_for(objects, objects.event_tracks, mappings.event_tracks, recipe.event_tracks);
    }

}
}