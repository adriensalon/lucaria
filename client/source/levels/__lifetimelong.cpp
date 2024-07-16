#include <entt/entt.hpp>

#include <glue/fetch.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/transform.hpp>

namespace detail {

static entt::entity register_player(entt::registry& registry)
{
    const entt::entity _player_entity = registry.create();

    // std::shared_future<std::shared_ptr<mesh_ref>> _mesh_ref = fetch_mesh("assets/decimategltf.bin");

    registry.emplace<model_component<model_shader::unlit>>(_player_entity)
        .mesh(fetch_mesh("assets/decimategltf.bin"))
        .material(fetch_material({{ material_texture::color, "assets/room_color.bin" }}));
    
    registry.emplace<transform_component>(_player_entity)
        .rotation_relative(glm::vec3(-1.57f, 0.f, 0.f))
        .position_warp(glm::vec3(3.f, 0.f, 0.f));
    
    // constexpr glm::uint _lolanim = 777;
    // registry.emplace<animator_component>(_player_entity)
    //     // .armature(fetch_armature("assets/lol_armature.bin"))
    //     // .moveset(fetch_moveset({{ _lolanim, "assets/lol_animation_AnimLol.bin" }}))
    //     .skeleton(fetch_skeleton("assets/lol_skeleton.bin"))
    //     .play(_lolanim);
    
    registry.emplace<collider_component<collider_detection::passive>>(_player_entity);
    
    registry.emplace<rigidbody_component>(_player_entity);
        // volume
        // weight

    return _player_entity;
}

}

static void register_lifetimelong(entt::registry& registry)
{
    const entt::entity _player_entity = detail::register_player(registry);
}