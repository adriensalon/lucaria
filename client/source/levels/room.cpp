#include <entt/entt.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/world.hpp>

#include <levels/levels.hpp>

void register_level_001_room(entt::registry& registry)
{
    const entt::entity _room_entity = registry.create();
    registry.emplace<model_component>(_room_entity)
        .mesh(std::move(fetch_mesh("assets/decimategltf.bin")))
        .texture(model_texture::color, std::move(fetch_texture("assets/room_color.bin")));
    
    std::future<animation_ref> _anim = fetch_animation("assets/eolienne_animation_wind_rotation.bin");
    std::future<skeleton_ref> _skeleton = fetch_skeleton("assets/decimategltf_skeleton.bin");
    // registry.emplace<collider_component>(_room_entity)
    //     .volume(std::move(fetch_volume("assets/decimategltf.bin")));

    // const entt::entity _speakers_entity = registry.create();
    // std::future<mesh_data> _speakers_mesh = fetch_mesh_data("assets/speakers_mesh.bin");
    // std::future<texture_data> _speakers_color = fetch_texture("assets/speakers_color.bin");
    // registry.emplace<model_component>(_speakers_entity, std::move(_speakers_mesh), std::move(_speakers_color));
    // registry.emplace<speaker_component>(_speakers_entity);
}