#include <fstream>

#define CEREAL_FUTURE_EXPERIMENTAL
#include <cereal/archives/adapters.hpp>

#include <lucaria/core/manager_game.hpp>
#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/utils_access.hpp>
#include <lucaria/core/utils_register.hpp>
#include <lucaria/public/component_animator.hpp>
#include <lucaria/public/component_interface.hpp>
#include <lucaria/public/component_model.hpp>
#include <lucaria/public/component_rigidbody.hpp>
#include <lucaria/public/component_speaker.hpp>
#include <lucaria/public/component_transform.hpp>
#include <lucaria/public/context_game.hpp>


namespace lucaria {
namespace detail {

    manager_game::manager_game()
    {
        context_game _game;
        access_context _access;
        _access.set(*this, _game);
        _access.set(input, _game.input);
        _access.set(objects, _game.objects);
        _access.set(scenes, _game.scenes);
        _access.set(window, _game.window);
        _access.set(dynamics, _game.dynamics);
        _access.set(mixer, _game.mixer);
        _access.set(rendering, _game.rendering);
        apply_scene_registrations(scenes);
        window.run(input, objects,

            [this, &_game]() { apply_main_scene(_game); }, [this, &_game]() {
			scenes.update_callbacks(_game);
			scenes.update_systems(*this); 
			objects.gc_unused(); });
    }

    void manager_game::save_snapshot(const std::filesystem::path& path)
    {
        recipe_manager_game _recipe_game = {};

        mappings_manager_game_save _mappings = {};
        _recipe_game.objects = make_recipe(objects, _mappings.objects);
        _recipe_game.scenes = make_recipe(scenes, _mappings.scenes);

        std::ofstream _ofstream(path, std::ios::binary);
        archive_json_output _archive(_mappings, _ofstream);
        _archive(cereal::make_nvp("game", _recipe_game));
    }

    void manager_game::load_snapshot(const std::filesystem::path& path)
    {
        // std::ifstream _ifstream(path, std::ios::binary);
        // // cereal::PortableBinaryInputArchive _archive(_ifstream);
        // cereal::JSONInputArchive _archive(_ifstream);

        // // objects
        // recipe_manager_object _save_recipes = {};
        // _archive(cereal::make_nvp("objects", _save_recipes));
        // object_save_database _save_objects = engine_resources().apply_recipes(std::move(_save_recipes));

        // // scenes
        // std::unordered_map<std::string, scene_type_implementation>& _scene_types = engine_scene_types();
        // for (scene_implementation& _scene : engine_scenes()) {
        //     // components TODO LATER

        //     scene_type_implementation& _scene_type = _scene_types.at(_scene.type_id);
        //     // _scene_type.binary_load(_scene, _archive);
        //     _scene_type.json_load(_scene, _archive);
        // }
    }

}
}
