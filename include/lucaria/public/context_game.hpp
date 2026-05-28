#pragma once

#include <lucaria/core/manager_game.hpp>
#include <lucaria/public/context_dynamics.hpp>
#include <lucaria/public/context_input.hpp>
#include <lucaria/public/context_mixer.hpp>
#include <lucaria/public/context_object.hpp>
#include <lucaria/public/context_rendering.hpp>
#include <lucaria/public/context_scene.hpp>
#include <lucaria/public/context_window.hpp>

namespace lucaria {

/// @brief Represents the context of the current scene, which can be used to create entities, add components and so on.
struct context_game {

    /// @brief
    context_input input;

    /// @brief
    context_object objects;

    /// @brief
    context_scene scenes;

    /// @brief
    context_dynamics dynamics;

    /// @brief
    context_mixer mixer;

    /// @brief
    context_rendering rendering;

    /// @brief
    context_window window;

    template <typename SceneType>
    SceneType& create_scene()
    {
        std::pair<SceneType&, detail::object_user_scene&> _scene = _manager->scenes.construct_scene<SceneType>();
		_manager->scenes.start_scene(*this, _scene.second);
		return _scene.first;
    }

    /// @brief
    /// @param path
    void save_snapshot(const std::filesystem::path& path);

    /// @brief
    /// @param path
    void load_snapshot(const std::filesystem::path& path);

private:
    detail::manager_game* _manager;
    friend struct access_context;
};

}
