#pragma once

#include <lucaria/core/utils_reload.hpp>
#include <lucaria/public/context_game.hpp>

#define LUCARIA_REGISTER_USER_ASSET_IMPLEMENTATION(AssetType)                          \
    struct AssetType##_user_asset_registration {                                       \
        AssetType##_user_asset_registration()                                          \
        {                                                                              \
            ::lucaria::detail::enqueue_user_asset_registration<AssetType>(#AssetType); \
        }                                                                              \
    };                                                                                 \
    static AssetType##_user_asset_registration AssetType##_user_asset_registration_instance;

#define LUCARIA_REGISTER_COMPONENT_IMPLEMENTATION(ComponentType)                              \
    struct ComponentType##_component_registration {                                           \
        ComponentType##_component_registration()                                              \
        {                                                                                     \
            ::lucaria::detail::enqueue_component_registration<ComponentType>(#ComponentType); \
        }                                                                                     \
    };                                                                                        \
    static ComponentType##_component_registration ComponentType##_component_registration_instance;

#define LUCARIA_REGISTER_SCENE_IMPLEMENTATION(SceneType)                          \
    struct SceneType##_scene_registration {                                       \
        SceneType##_scene_registration()                                          \
        {                                                                         \
            ::lucaria::detail::enqueue_scene_registration<SceneType>(#SceneType); \
        }                                                                         \
    };                                                                            \
    static SceneType##_scene_registration SceneType##_scene_registration_instance;

#define LUCARIA_MAIN_SCENE_IMPLEMENTATION(SceneType)                                         \
    struct SceneType##_main_scene_registration {                                             \
        SceneType##_main_scene_registration()                                                \
        {                                                                                    \
            ::lucaria::detail::enqueue_main_scene_registration<SceneType>();                 \
        }                                                                                    \
    };                                                                                       \
    static SceneType##_main_scene_registration SceneType##_main_scene_registration_instance; \
    LUCARIA_RELOAD_MODULE_IMPLEMENTATION

namespace lucaria {

struct context_game;

namespace detail {

    struct pending_user_asset_registration {
        std::string type_id;
        void (*register_into)(manager_object&, std::string);
    };

    struct pending_component_registration {
        std::string type_id;
        void (*register_into)(manager_scene&, std::string);
    };

    struct pending_scene_registration {
        std::string type_id;
        void (*register_into)(manager_scene&, std::string);
    };

    struct pending_main_scene_registration {
        void (*emplace_into)(context_game&);
    };

    inline std::vector<pending_user_asset_registration>& global_pending_user_asset_registrations()
    {
        static std::vector<pending_user_asset_registration> _assets = {};
        return _assets;
    }

    inline std::vector<pending_component_registration>& global_pending_component_registrations()
    {
        static std::vector<pending_component_registration> _components = {};
        return _components;
    }

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

    template <typename AssetType>
    void enqueue_user_asset_registration(std::string type_id)
    {
        global_pending_user_asset_registrations().push_back({ std::move(type_id),
            [](manager_object& objects, std::string type_id) {
                objects.register_user_asset<AssetType>(std::move(type_id));
            } });
    }

    template <typename ComponentType>
    void enqueue_component_registration(std::string type_id)
    {
        global_pending_component_registrations().push_back({ std::move(type_id),
            [](manager_scene& scenes, std::string type_id) {
                scenes.register_component_user<ComponentType>(std::move(type_id));
            } });
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

    inline void apply_user_asset_registrations(manager_object& objects)
    {
        for (pending_user_asset_registration& _asset : global_pending_user_asset_registrations()) {
            _asset.register_into(objects, _asset.type_id);
        }
    }

    inline void apply_component_registrations(manager_scene& scenes)
    {
        for (pending_component_registration& _component : global_pending_component_registrations()) {
            _component.register_into(scenes, _component.type_id);
        }
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

    inline void clear_pending_type_registrations()
    {
        global_pending_user_asset_registrations().clear();
        global_pending_component_registrations().clear();
        global_pending_scene_registrations().clear();
    }

    inline void clear_pending_main_scene_registration()
    {
        global_pending_main_scene_registration() = {};
    }

}
}