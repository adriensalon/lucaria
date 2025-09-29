#include <lucaria/core/world.hpp>

namespace lucaria {
namespace detail {

    void set_scenes(std::vector<entt::registry>& scenes)
    {
        global_scenes = &scenes;
    }

    void each_scene(const std::function<void(entt::registry&)>& callback)
    {
        if (global_scenes) {
            for (entt::registry& _scene : *global_scenes) {
                callback(_scene);
            }
        }
    }

    void destroy_scenes()
    {
        if (global_scenes) {
            for (entt::registry& _scene : *global_scenes) {
                _scene.clear();
            }
        }
    }

    std::size_t get_scenes_count()
    {
        return global_scenes ? global_scenes->size() : 0;
    }

}
}
