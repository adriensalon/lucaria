#include <core/world.hpp>
#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>

struct environment_static_actor {
    environment_static_actor(scene_data& scene, const std::string& uuid);
    collider_component& get_collider();

private:
    std::optional<std::reference_wrapper<collider_component>> _collider = std::nullopt;
};