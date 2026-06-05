#include <fstream>

#define CEREAL_FUTURE_EXPERIMENTAL
#include <cereal/archives/adapters.hpp>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_game.hpp>
#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/game_access.hpp>
#include <lucaria/core/game_register.hpp>
#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_interface.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/component_transform.hpp>
#include <lucaria/forward/context_game.hpp>

// Store static pointers when hotreloading is enabled
LUCARIA_PLUGIN_EXPORT bool __lucaria_plugin_register(::lucaria::detail::manager_game* game);
LUCARIA_PLUGIN_EXPORT bool __lucaria_plugin_start(::lucaria::context_game* game);

namespace lucaria {
namespace detail {

    manager_game::manager_game()
    {
        context_game _game;
        context = &_game;

        access_context _access;
        _access.set(*this, _game);
        _access.set(input, _game.input);
        _access.set(objects, _game.objects);
        _access.set(scenes, _game.scenes);
        _access.set(window, _game.window);
        _access.set(dynamics, _game.dynamics);
        _access.set(mixer, _game.mixer);
        _access.set(rendering, _game.rendering);

        __lucaria_plugin_register(this);

        window.run(input, objects,

            [this, &_game]() { 
				
			// compile shaders
			scenes.compiler = std::make_unique<gsl_compiler>(scenes.gsl_systems);
			__lucaria_plugin_start(&_game); },

            [this, &_game]() {
			scenes.update_callbacks(_game);
			scenes.update_systems(*this); 
			objects.gc_unused(); });
    }

    void manager_game::save_snapshot(const std::filesystem::path& path)
    {
        mappings_manager_game_save _mappings = {};
        _mappings.saving_objects = &objects;

        snapshot_assets _objects { objects };
        snapshot_scenes _scenes { scenes };

        std::ofstream _ofstream(path, std::ios::binary);
        archive_json_output _archive(_mappings, _ofstream);
        _archive(cereal::make_nvp("assets", _objects));
        _archive(cereal::make_nvp("components", _scenes));
    }

    void manager_game::load_snapshot(const std::filesystem::path& path)
    {
        mappings_manager_game_load _mappings = {};
        _mappings.loading_objects = &objects;
        _mappings.loading_window = &window;
        _mappings.loading_scene_manager = &scenes;
        _mappings.dynamics = &context->dynamics;

        std::ifstream _ifstream(path, std::ios::binary);
        archive_json_input _archive(_mappings, _ifstream);

        snapshot_assets _objects { objects };
        snapshot_scenes _scenes { scenes };

        _archive(cereal::make_nvp("assets", _objects));
        _archive(cereal::make_nvp("components", _scenes));
    }

}
}
