#include <entt/entt.hpp>

#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/system/world.hpp>

#include <levels/levels.hpp>

void register_level_001_room(entt::registry& registry)
{
    const entt::entity _room_entity = registry.create();
    std::future<mesh_data> _room_mesh = fetch_mesh("assets/room_mesh.bin");
    std::future<texture_data> _room_color = fetch_texture("assets/room_color.bin");
    // std::future<volume_data> _room_nav = fetch_volume({ "assets/collision_mesh_1.bin", "assets/collision_mesh_2.bin" });
    registry.emplace<model_component>(_room_entity, std::move(_room_mesh), std::move(_room_color));
    // registry.emplace<collider_component>(_room_entity, std::move(_room_nav));

    // const entt::entity _speakers_entity = registry.create();
    // std::future<mesh_data> _speakers_mesh = fetch_mesh("assets/speakers_mesh.bin");
    // std::future<texture_data> _speakers_color = fetch_texture("assets/speakers_color.bin");
    // registry.emplace<model_component>(_speakers_entity, std::move(_speakers_mesh), std::move(_speakers_color));
    // registry.emplace<speaker_component>(_speakers_entity);
}