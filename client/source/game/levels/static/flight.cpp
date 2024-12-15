
#include <entt/entt.hpp>

#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>

#define GAME_OBJECT(uuid)                                                \
    const entt::entity go_##uuid = registry.create();                    \
    registry.emplace<model_component<model_shader::unlit>>(go_##uuid)    \
        .color(fetch_texture("assets/level/flight/image_" #uuid ".bin")) \
        .mesh(fetch_mesh("assets/level/flight/geometry_" #uuid ".bin"));

void level_static_flight(entt::registry& registry)
{
    GAME_OBJECT(fXbl)
    GAME_OBJECT(8Ijp)
    GAME_OBJECT(UQZZ)

    registry.emplace<collider_component>(go_fXbl)
        .shape(fetch_shape("assets/level/flight/shape_fXbl.bin", shape_type::triangle_mesh))
        .wall();
}