#pragma once

#include <lucaria/core/database.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/input.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/entity/dynamics.hpp>
#include <lucaria/entity/mixer.hpp>
#include <lucaria/entity/rendering.hpp>
#include <lucaria/entity/scene.hpp>

#define LUCARIA_STRINGIFY_IMPL(x) #x
#define LUCARIA_STRINGIFY(x) LUCARIA_STRINGIFY_IMPL(x)
#define LUCARIA_CONCAT_IMPL(a, b) a##b
#define LUCARIA_CONCAT(a, b) LUCARIA_CONCAT_IMPL(a, b)

/// @brief
#define LUCARIA_REGISTER_SCENE(SceneType)                                              \
    inline bool LUCARIA_CONCAT(__lucaria_scene_, SceneType) = [] { \
        lucaria::detail::register_scene_type<SceneType>(LUCARIA_STRINGIFY(SceneType)); \
        return true;                                                                   \
    }();

/// @brief
#define LUCARIA_MAIN_SCENE(SceneType)                                 \
    extern "C" void __lucaria_main_scene()                            \
    {                                                                 \
        lucaria::detail::engine_context().emplace_scene<SceneType>(); \
    }

namespace lucaria {

/// @brief Represents the context of the current scene, which can be used to create entities, add components and so on.
struct game_context {

    /// @brief Creates a new scene and returns its object
    /// @return the new scene object
    template <typename SceneType>
    SceneType& emplace_scene()
    {
        const std::string _type_id = detail::engine_scene_type_ids().at(std::type_index(typeid(SceneType)));
        detail::scene_type_implementation& _scene_type = detail::engine_scene_types().at(_type_id);
        detail::scene_implementation& _scene = detail::engine_scenes().emplace_back();
        _scene.type_id = _type_id;
        SceneType& _typed_scene = _scene.user_data.emplace<SceneType>();
		detail::engine_context().scene._self_scene = &_scene;
        if (_scene_type.start) {
            _scene_type.start(_scene);
        }

        return _typed_scene;
    }

    template <typename SceneType>
    SceneType& emplace_scene(SceneType&& scene)
    {
        const std::string _type_id = detail::engine_scene_type_ids().at(std::type_index(typeid(SceneType)));
        detail::scene_type_implementation& _scene_type = detail::engine_scene_types().at(_type_id);
        detail::scene_implementation& _scene = detail::engine_scenes().emplace_back();
        _scene.type_id = _type_id;
        SceneType& _typed_scene = _scene.user_data.emplace<SceneType>(std::move(scene));
		detail::engine_context().scene._self_scene = &_scene;
        if (_scene_type.start) {
            _scene_type.start(_scene);
        }

        return _typed_scene;
    }

    /// @brief
    /// @param path
    void save_snapshot(const std::filesystem::path& path);

    /// @brief
    /// @param path
    void load_snapshot(const std::filesystem::path& path);

    /// @brief
    input_context input;

    /// @brief
    fetch_context fetch;

    /// @brief
    object_context object;

    /// @brief
    scene_context scene;

    /// @brief
    dynamics_context dynamics;

    /// @brief
    mixer_context mixer;

    /// @brief
    rendering_context rendering;

	window_context window;
};

}