#pragma once

#include <functional>
#include <iostream>
#include <vector>
#include <set>
#include <any>
#include <typeinfo>

#include <entt/entt.hpp>

struct scene_data {

    entt::registry components = {};

    template <typename actor_t, typename... args_t>
    actor_t& make_actor(args_t&&... args)
    {
        entt::entity _entity = actors_registry.create();
        return actors_registry.emplace<actor_t>(_entity, std::forward<args_t>(args)...);
    }

    template <typename actor_t>
    void destroy_actor(actor_t& actor)
    {
        actor_t* _actor_ptr = std::addressof(actor);
        std::optional<entt::entity> _found_entity = std::nullopt;
        actors_registry.view<actor_t>().each([&_found_entity](const entt::entity& existing_entity, actor_t& existing_actor) {
            actor_t* _existing_actor_ptr = std::addressof(existing_actor);
            if (_actor_ptr == _existing_actor_ptr) {
                _found_entity = existing_entity;
                return;
            }
        });
        if (_found_entity.has_value()) {
            actors_registry.erase<actor_t>(_found_entity.value());
        }
    }
    
    template <typename actor_t>
    void each_actor(const std::function<void(actor_t&)>& callback)
    {
        actors_registry.view<actor_t>().each([callback](actor_t& _actor) {
            callback(_actor);
        });
    }

    template <typename actor_t>
    void update_actors()
    {
        each_actor<actor_t>([](actor_t& _actor) {
            _actor.update();
        });
    }

private:
    entt::registry actors_registry = {};
};


namespace detail {

inline std::vector<scene_data> world_scenes = {};
inline std::vector<std::any> world_data = {};
inline std::set<std::string> world_types = {};

}

template <typename scene_t>
void make_scene()
{
    std::string _world_type(typeid(scene_t).name());
    if (detail::world_types.count(_world_type) > 0) {
        std::cout << "only one scene of each type" << std::endl;
        std::terminate();
    }
    scene_data& _data = detail::world_scenes.emplace_back();
    std::cout << "scene data created \n";
    detail::world_data.emplace_back((std::make_any<scene_t>(_data)));
    detail::world_types.emplace(_world_type);
    std::cout << "scene created \n";
}

template <typename scene_t>
void destroy_scene()
{
    // if (detail::world_types.find(typeid(scene_t) == detail::world_types.end())) {
    //     std::cout << "only one scene of each type" << std::endl;
    //     std::terminate();
    // }
    // auto _view = detail::scenes_registry.view<scene_t>();
    // detail::scenes_registry.erase(_view.begin(), _view.end());
}

void each_scene(const std::function<void(scene_data&)>& callback);
