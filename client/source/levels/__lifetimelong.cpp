#include <entt/entt.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/collider.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/rigidbody.hpp>
#include <ecs/component/speaker.hpp>
#include <ecs/component/transform.hpp>

namespace detail {

static entt::entity register_player(entt::registry& registry)
{
    const entt::entity _player_entity = registry.create();
    
    registry.emplace<model_component>(_player_entity)
        .mesh(std::move(fetch_mesh("assets/lol.bin", true)))
        .texture(model_texture::color, std::move(fetch_texture("assets/room_color.bin")));
    
    registry.emplace<transform_component>(_player_entity)
        .position_warp(glm::vec3(3.f, 0.f, 0.f));
    
    registry.emplace<animator_component>(_player_entity)
        .skeleton(std::move(fetch_skeleton("assets/lol_skeleton.bin")))
        .animation("lol", std::move(fetch_animation("assets/lol_animation_AnimLol.bin")))
        .play("lol");
    
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