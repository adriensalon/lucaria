#pragma once

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/public/handle_entity.hpp>

namespace lucaria {

/// @brief
struct context_scene {

    /// @brief Creates a new entity in the scene and returns its handle
    /// @return the new entity handle
    [[nodiscard]] handle_entity create_entity();

    /// @brief Schedules an entity for erasure at the end of the current frame. The entity will be removed from the scene and all its components will be freed.
    /// @param entity
    void mark_erase_entity(const handle_entity entity);

    /// @brief Schedules the current scene for erasure at the end of the current frame. The scene will be removed from the engine and all its resources will be freed.
    void mark_erase_self();

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam Callback
    /// @param callback
    template <typename... ComponentTypes, typename Callback>
    void each_view(Callback&& callback)
    {
        _manager->each_view<ComponentTypes...>(std::forward<Callback>(callback));
    }

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam ...ExcludeComponentTypes
    /// @tparam Callback
    /// @param exclude
    /// @param callback
    template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
    void each_view(detail::exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
    {
        _manager->each_view<ComponentTypes...>(exclude, std::forward<Callback>(callback));
    }

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam Callback
    /// @param callback
    // template <typename... ComponentTypes, typename Callback>
    // void each_view_self(Callback&& callback)
    // {
    //     _manager->current_scene->components.view<ComponentTypes...>().each(std::forward<Callback>(callback));
    // }

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam ...ExcludeComponentTypes
    /// @tparam Callback
    /// @param exclude
    /// @param callback
    // template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
    // void each_view_self(detail::exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
    // {
    //     _manager->current_scene->components.view<ComponentTypes...>(exclude).each(std::forward<Callback>(callback));
    // }

    /// @brief
    /// @param entity
    /// @return
    component_animator& create_animator(const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_interface_screen& create_screen_interface(const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_interface_spatial& create_spatial_interface(const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_model_blockout& create_blockout_model(const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_model_unlit& create_unlit_model(const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_rigidbody_passive& create_passive_rigidbody(context_dynamics& dynamics, const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_rigidbody_kinematic& create_kinematic_rigidbody(context_dynamics& dynamics, const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_rigidbody_dynamic& create_dynamic_rigidbody(context_dynamics& dynamics, const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_speaker_spatial& create_speaker(const handle_entity entity);

    /// @brief
    /// @param entity
    /// @return
    component_transform& create_transform(const handle_entity entity);

    /// @brief Creates a registered user component on an entity.
    /// @tparam ComponentType User component type registered with manager_scene::register_user_component.
    /// @tparam ...Args Constructor argument types.
    /// @param entity Target entity.
    /// @param ...args Constructor arguments forwarded to the component.
    /// @return the newly created user component.
    template <typename ComponentType, typename... Args>
    ComponentType& create_component_user(const handle_entity entity, Args&&... args)
    {
        return _manager->segment_registry_cpu.emplace<ComponentType>(entity._entity, std::forward<Args>(args)...);
    }

private:
    detail::manager_scenes* _manager;
	friend struct access_context;
};

}
