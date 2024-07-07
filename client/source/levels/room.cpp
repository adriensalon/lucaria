#include <entt/entt.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/world.hpp>

#include <levels/levels.hpp>

void register_level_001_room(entt::registry& registry)
{
    const entt::entity _room_entity = registry.create();
    
    registry.emplace<model_component>(_room_entity)
        .mesh(std::move(fetch_mesh("assets/lol.bin", true)))
        .texture(model_texture::color, std::move(fetch_texture("assets/room_color.bin")));
    
    registry.emplace<transform_component>(_room_entity)
        .position_warp(glm::vec3(3.f, 0.f, 0.f));
    
    registry.emplace<animator_component>(_room_entity)
        .skeleton(std::move(fetch_skeleton("assets/lol_skeleton.bin")))
        .animation("lol", std::move(fetch_animation("assets/lol_animation_AnimLol.bin")))
        .play("lol");
        

    // const entt::entity _speakers_entity = registry.create();
    // std::future<mesh_data> _speakers_mesh = fetch_mesh_data("assets/speakers_mesh.bin");
    // std::future<texture_data> _speakers_color = fetch_texture("assets/speakers_color.bin");
    // registry.emplace<model_component>(_speakers_entity, std::move(_speakers_mesh), std::move(_speakers_color));
    // registry.emplace<speaker_component>(_speakers_entity);
}