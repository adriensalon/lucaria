#pragma once

#include <any>
#include <functional>
#include <iostream>
#include <set>
#include <typeinfo>
#include <vector>

#include <entt/entt.hpp>

namespace lucaria {

struct scene_data {

    scene_data() = default;
    scene_data(const scene_data& other) = delete;
    scene_data& operator=(const scene_data& other) = delete;
    scene_data(scene_data&& other) = default;
    scene_data& operator=(scene_data&& other) = default;
    inline ~scene_data()
    {
        actors_registry.clear();
        components.clear();
    }

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
    inline std::vector<std::function<void()>> manage_callbacks = {};

}

template <typename scene_t>
void make_scene()
{
    detail::manage_callbacks.emplace_back([]() {
        std::string _world_type(typeid(scene_t).name());
        if (detail::world_types.count(_world_type) > 0) {
            std::cout << "only one scene of each type" << std::endl;
            std::terminate();
        }
        scene_data& _data = detail::world_scenes.emplace_back();
        // std::cout << "scene data created \n";
        detail::world_data.emplace_back((std::make_any<scene_t>(_data)));
        detail::world_types.emplace(_world_type);
        // std::cout << "scene created \n";
    });
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

inline void destroy_scenes()
{
    detail::manage_callbacks.clear();
    std::cout << "Destroyed world pending manage callbacks" << std::endl;
    detail::world_types.clear();
    std::cout << "Destroyed world types list" << std::endl;
    detail::world_scenes.clear();
    std::cout << "Destroyed world scenes" << std::endl;
    detail::world_data.clear();
    std::cout << "Destroyed world actors" << std::endl;
    std::cout << "Destroyed world components" << std::endl;
}

void each_scene(const std::function<void(scene_data&)>& callback);

inline void manage()
{
    for (std::function<void()>& _manage_callback : detail::manage_callbacks) {
        _manage_callback();
        std::cout << "Manage cb^^" << std::endl;
    }
    detail::manage_callbacks.clear();
}

inline std::size_t get_scenes_count()
{
    return detail::world_scenes.size();
}

}