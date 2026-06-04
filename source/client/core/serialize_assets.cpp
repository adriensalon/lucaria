#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/serialize_assets.hpp>

namespace lucaria {
namespace detail {

    recipe_manager_object make_recipe(manager_assets& objects, mappings_manager_object_save& mappings)
    {
        recipe_manager_object _storage;

        make_storage_entries_for(_storage.images, objects, objects.images, mappings.images);
        make_storage_entries_for(_storage.textures, objects, objects.textures, mappings.textures);
        make_storage_entries_for(_storage.cubemaps, objects, objects.cubemaps, mappings.cubemaps);
        make_storage_entries_for(_storage.geometries, objects, objects.geometries, mappings.geometries);
        make_storage_entries_for(_storage.shapes, objects, objects.shapes, mappings.shapes);
        make_storage_entries_for(_storage.meshes, objects, objects.meshes, mappings.meshes);
        make_storage_entries_for(_storage.fonts, objects, objects.fonts, mappings.fonts);
        make_storage_entries_for(_storage.audios, objects, objects.audios, mappings.audios);
        make_storage_entries_for(_storage.sound_tracks, objects, objects.sound_tracks, mappings.sound_tracks);
        make_storage_entries_for(_storage.skeletons, objects, objects.skeletons, mappings.skeletons);
        make_storage_entries_for(_storage.animations, objects, objects.animations, mappings.animations);
        make_storage_entries_for(_storage.motion_tracks, objects, objects.motion_tracks, mappings.motion_tracks);
        make_storage_entries_for(_storage.event_tracks, objects, objects.event_tracks, mappings.event_tracks);

        for (const auto& [_type_id, _callbacks] : objects.user_asset_types) {
            storage_user_asset_group& _group = _storage.user_assets.emplace_back();
            _group.type_id = _type_id;
            _group.objects = const_cast<manager_assets*>(&objects);
            _group.callbacks = const_cast<user_asset_type_callbacks*>(&_callbacks);
        }

        return _storage;
    }

    void apply_recipe(manager_window&, manager_assets&, mappings_manager_object_load&, recipe_manager_object&)
    {
        // Asset cells are created directly by storage_asset_entry::load while the archive is read.
    }

}
}