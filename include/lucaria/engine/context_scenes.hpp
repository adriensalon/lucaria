#pragma once

#include <type_traits>
#include <utility>

#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/scenes_view.hpp>
#include <lucaria/core/user_components.hpp>
#include <lucaria/engine/handle_entity.hpp>
#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_interface.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/component_transform.hpp>

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

    template <auto SystemFunction>
    void run_dispatch_compute()
    {
        _manager->run_dispatch_compute<SystemFunction>();
    }

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
		static_assert(!::lucaria::traits::component_compute_enable_v<ComponentType>, "Compute user components must be created with emplace_compute_user");
        return _manager->registry.emplace<ComponentType>(entity._entity, std::forward<Args>(args)...);
    }

    template <typename ComponentType, typename... Args>
    void emplace_compute_user(const handle_entity entity, Args&&... args)
    {
		static_assert(::lucaria::traits::component_compute_enable_v<ComponentType>, "Standard user components must be created with emplace_user");
        _manager->registry.emplace_compute<ComponentType>(entity._entity, std::forward<Args>(args)...);
    }

	

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_animator& get_animator(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_interface_screen& get_screen_interface(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_interface_spatial& get_spatial_interface(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_model_blockout& get_blockout_model(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_model_unlit& get_unlit_model(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_rigidbody_passive& get_passive_rigidbody(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_rigidbody_kinematic& get_kinematic_rigidbody(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_rigidbody_dynamic& get_dynamic_rigidbody(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_speaker_spatial& get_speaker(const handle_entity entity) const;

    /// @brief
    /// @param entity
    /// @return
    [[nodiscard]] component_transform& get_transform(const handle_entity entity) const;

    /// @brief Returns true if this entity owns the requested user component.
    /// @tparam ComponentType User component type.
    /// @return true if the entity has the component, false otherwise.
    template <typename ComponentType>
    bool has_component_user(const handle_entity entity) const
    {
        return _manager->registry.contains<ComponentType>(entity._entity);
    }

    /// @brief Gets a user component from this entity.
    /// @tparam ComponentType User component type.
    template <typename ComponentType>
    [[nodiscard]] ComponentType& get_component_user(const handle_entity entity) const
    {
        return _manager->registry.get<ComponentType>(entity._entity);
    }

    /// @brief Removes a user component from this entity if present.
    /// @tparam ComponentType User component type.
    template <typename ComponentType>
    void remove_component_user(const handle_entity entity) const
    {
        if (_manager) {
            _manager->registry.remove<ComponentType>(entity._entity);
        }
    }

private:
    detail::manager_scenes* _manager;
    friend struct access_context;
};

}
