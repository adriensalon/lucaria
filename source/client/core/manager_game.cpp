#include <fstream>

#define CEREAL_FUTURE_EXPERIMENTAL
#include <cereal/archives/adapters.hpp>

#include <lucaria/core/game_access.hpp>
#include <lucaria/core/game_register.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_game.hpp>
#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_interface.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/component_transform.hpp>
#include <lucaria/engine/context_game.hpp>

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

#if !defined(LUCARIA_DISABLE_RELOAD)
        user_module.module_register(this);
#else
        __lucaria_plugin_register(this);
#endif

        window.run(input, objects,

            [this, &_game]() {

#if !defined(LUCARIA_DISABLE_COMPUTE_SPIRV)
                scenes.compiler = std::make_unique<gsl_compiler>(scenes.gsl_systems);
#endif
#if !defined(LUCARIA_DISABLE_RELOAD)
                user_module.module_start(&_game);
#else
                __lucaria_plugin_start(&_game);
#endif
            },

            [this, &_game]() {
                scenes.update_callbacks(_game);
                scenes.update_systems(*this);
                objects.gc_unused();

                int _cmake_return;
                std::string _cmake_output;
                object_reload_module_status _status = user_module.poll_sources_and_recompile_library(_cmake_return, _cmake_output);
                if (_status == object_reload_module_status::compilation_started) {
                    std::cout << "compilation started" << std::endl;
                }
                if (_status == object_reload_module_status::compilation_finished) {
                    std::cout << _cmake_output << std::endl;

                    if (_cmake_return != 0) {
                        return;
                    }

                    const std::filesystem::path _snapshot_path = user_module.cache_directory / "reload_snapshot.json";
                    hot_reload(_snapshot_path);
                }
            });
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
        _archive(cereal::make_nvp("rendering", rendering));
        _archive(cereal::make_nvp("dynamics", dynamics));
        _archive(cereal::make_nvp("mixer", mixer));
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
        _archive(cereal::make_nvp("rendering", rendering));
        _archive(cereal::make_nvp("dynamics", dynamics));
        _archive(cereal::make_nvp("mixer", mixer));

        rendering.resolve_runtime_references(scenes);
        mixer.resolve_runtime_references(scenes);
        dynamics.apply_runtime_settings();
    }

#if !defined(LUCARIA_DISABLE_RELOAD)
    bool manager_game::hot_reload(const std::filesystem::path& snapshot_path)
    {
        save_snapshot(snapshot_path);

        std::string _reload_output = {};
        std::optional<object_reload_candidate> _candidate = user_module.load_next_library(_reload_output);
        if (!_candidate) {
            std::cout << _reload_output << std::endl;
            return false;
        }

        clear_runtime_for_reload();

        if (!_candidate->module_register(this)) {
            std::cout << "__lucaria_plugin_register failed" << std::endl;
            return false;
        }

#if !defined(LUCARIA_DISABLE_COMPUTE_SPIRV)
        scenes.compiler = std::make_unique<gsl_compiler>(scenes.gsl_systems);
#endif

        user_module.commit_library(std::move(*_candidate));
        load_snapshot(snapshot_path);

        return true;
    }

    void manager_game::clear_runtime_for_reload()
    {
        rendering.clear_runtime_references_for_reload();
        scenes.clear_runtime_for_reload(*context);
        objects.clear_runtime_for_reload();
        scenes.clear_plugin_registrations_for_reload();
    }
#endif
}
}
