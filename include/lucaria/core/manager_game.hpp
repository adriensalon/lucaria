#pragma once

#include <lucaria/core/manager_input.hpp>
#include <lucaria/core/manager_object.hpp>
#include <lucaria/core/manager_scene.hpp>
#include <lucaria/core/manager_window.hpp>
#include <lucaria/core/system_dynamics.hpp>
#include <lucaria/core/system_motion.hpp>
#include <lucaria/core/system_mixer.hpp>
#include <lucaria/core/system_rendering.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    struct manager_game {
        manager_game();
        manager_game(const manager_game& other) = delete;
        manager_game& operator=(const manager_game& other) = delete;
        manager_game(manager_game&& other) = default;
        manager_game& operator=(manager_game&& other) = default;

        manager_input input = {};
        manager_object objects = {};
        manager_scene scenes = {};
        manager_window window = {};
		system_dynamics dynamics = {};
		system_motion motion = {};
		system_mixer mixer = {};
		system_rendering rendering = {};

        void save_snapshot(const std::filesystem::path& path);
        void load_snapshot(const std::filesystem::path& path);
    };

    // recipes

    struct recipe_manager_game {
        recipe_manager_object objects = {};
        recipe_manager_scene scenes = {};
        recipe_manager_window window = {};
        // etc

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("objects", objects));
            archive(cereal::make_nvp("scenes", scenes));
            archive(cereal::make_nvp("window", window));
            // etc
        }
    };

    // [[nodiscard]] recipe_manager_game make_recipe(mappings_manager_game& mappings);

}
}
