#pragma once

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_input.hpp>
#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/manager_window.hpp>
#include <lucaria/core/serialize_archives.hpp>
#include <lucaria/core/serialize_assets.hpp>
#include <lucaria/core/serialize_containers.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/serialize_scenes.hpp>
#include <lucaria/core/system_dynamics.hpp>
#include <lucaria/core/system_mixer.hpp>
#include <lucaria/core/system_motion.hpp>
#include <lucaria/core/system_rendering.hpp>

namespace lucaria {

struct context_game;

namespace detail {

    struct manager_game {
        manager_game();
        manager_game(const manager_game& other) = delete;
        manager_game& operator=(const manager_game& other) = delete;
        manager_game(manager_game&& other) = delete;
        manager_game& operator=(manager_game&& other) = delete;

        manager_input input = {};
        manager_assets objects = {};
        manager_scenes scenes = {};
        manager_window window = {};
        system_dynamics dynamics = {};
        system_motion motion = {};
        system_mixer mixer = {};
        system_rendering rendering = {};
        context_game* context = nullptr;

        void save_snapshot(const std::filesystem::path& path);
        void load_snapshot(const std::filesystem::path& path);
    };

}
}
