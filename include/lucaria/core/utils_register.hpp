#pragma once

#include <lucaria/core/manager_scene.hpp>
#include <lucaria/public/context_game.hpp>

#define LUCARIA_REGISTER_SCENE_IMPLEMENTATION(SceneType)                          \
    struct SceneType##_scene_registration {                                       \
        SceneType##_scene_registration()                                          \
        {                                                                         \
            ::lucaria::detail::enqueue_scene_registration<SceneType>(#SceneType); \
        }                                                                         \
    };                                                                            \
    static SceneType##_scene_registration SceneType##_scene_registration_instance;

#define LUCARIA_MAIN_SCENE_IMPLEMENTATION(SceneType)                         \
    struct SceneType##_main_scene_registration {                             \
        SceneType##_main_scene_registration()                                \
        {                                                                    \
            ::lucaria::detail::enqueue_main_scene_registration<SceneType>(); \
        }                                                                    \
    };                                                                       \
    static SceneType##_main_scene_registration SceneType##_main_scene_registration_instance;

namespace lucaria {

struct context_game;

namespace detail {

    struct pending_scene_registration {
        std::string type_id;
        void (*register_into)(manager_scene&, std::string);
    };

    struct pending_main_scene_registration {
        void (*emplace_into)(context_game&);
    };

    inline std::vector<pending_scene_registration>& global_pending_scene_registrations()
    {
        static std::vector<pending_scene_registration> _scenes = {};
        return _scenes;
    }

    inline pending_main_scene_registration& global_pending_main_scene_registration()
    {
        static pending_main_scene_registration _main_scene = {};
        return _main_scene;
    }

    template <typename SceneType>
    void enqueue_scene_registration(std::string type_id)
    {
        global_pending_scene_registrations().push_back({ std::move(type_id),
            [](manager_scene& scenes, std::string type_id) {
                scenes.register_scene<SceneType>(std::move(type_id));
            } });
    }

    template <typename SceneType>
    void enqueue_main_scene_registration()
    {
        global_pending_main_scene_registration() = {
            [](context_game& game) {
                game.template create_scene<SceneType>();
            }
        };
    }

    inline void apply_scene_registrations(manager_scene& scenes)
    {
        for (pending_scene_registration& _scene : global_pending_scene_registrations()) {
            _scene.register_into(scenes, _scene.type_id);
        }
    }

    inline void apply_main_scene(context_game& game)
    {
        pending_main_scene_registration& _main_scene = global_pending_main_scene_registration();

        if (_main_scene.emplace_into) {
            _main_scene.emplace_into(game);
        }
    }

}
}