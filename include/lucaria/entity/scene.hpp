#pragma once

#include <any>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <entt/entt.hpp>
#include <pfr.hpp>

#include <lucaria/core/detection.hpp>
#include <lucaria/entity/animator.hpp>
#include <lucaria/entity/interface.hpp>
#include <lucaria/entity/model.hpp>
#include <lucaria/entity/rigidbody.hpp>
#include <lucaria/entity/speaker.hpp>
#include <lucaria/entity/transform.hpp>

template <typename Archive, typename SceneType>
std::enable_if_t<std::is_aggregate_v<SceneType>>
serialize(Archive& archive, SceneType& object)
{
    pfr::for_each_field_with_name(object, [&](std::string_view name, auto& field) {
        archive(cereal::make_nvp(std::string(name).c_str(), field));
    });
}

namespace lucaria {

struct game_context;

/// @brief
using scene_entity = entt::entity;

/// @brief
// #define scene_exclude = entt::exclude;

namespace detail {

    struct scene_implementation {
        std::string type_id = {};
        bool is_marked_erase = false;
        std::vector<entt::entity> entities_marked_erase = {};
        entt::registry components = {};
        std::any user_data = {};
    };

    struct scene_type_implementation {
        std::function<void(scene_implementation&)> construct = nullptr;
        std::function<void(scene_implementation&)> start = nullptr;
        std::function<void(scene_implementation&)> update = nullptr;
        std::function<void(scene_implementation&)> stop = nullptr;
        std::function<void(scene_implementation&, cereal::PortableBinaryOutputArchive&)> binary_save = nullptr;
        std::function<void(scene_implementation&, cereal::PortableBinaryInputArchive&)> binary_load = nullptr;
        std::function<void(scene_implementation&, cereal::JSONOutputArchive&)> json_save = nullptr;
        std::function<void(scene_implementation&, cereal::JSONInputArchive&)> json_load = nullptr;
    };

    [[nodiscard]] std::vector<scene_implementation>& engine_scenes();
    [[nodiscard]] std::unordered_map<std::string, scene_type_implementation>& engine_scene_types();
    [[nodiscard]] std::unordered_map<std::type_index, std::string>& engine_scene_type_ids();
    [[nodiscard]] game_context& engine_context();

    template <typename SceneType>
    void register_scene_type(std::string type_id)
    {
        scene_type_implementation _scene_type = {};

        _scene_type.construct = [](scene_implementation& scene) {
            scene.user_data.emplace<SceneType>();
        };

        if constexpr (has_start_v<SceneType>) {
            _scene_type.start = [](scene_implementation& scene) {
                SceneType& typed_scene = std::any_cast<SceneType&>(scene.user_data);
                typed_scene.start(engine_context());
            };
        }
        if constexpr (has_update_v<SceneType>) {
            _scene_type.update = [](scene_implementation& scene) {
                SceneType& typed_scene = std::any_cast<SceneType&>(scene.user_data);
                typed_scene.update(engine_context());
            };
        }
        if constexpr (has_stop_v<SceneType>) {
            _scene_type.stop = [](scene_implementation& scene) {
                SceneType& typed_scene = std::any_cast<SceneType&>(scene.user_data);
                typed_scene.stop(engine_context());
            };
        }

        _scene_type.binary_save = [](scene_implementation& scene, cereal::PortableBinaryOutputArchive& archive) {
            SceneType& typed_scene = std::any_cast<SceneType&>(scene.user_data);
            // archive(typed_scene);
        };

        _scene_type.binary_load = [](scene_implementation& scene, cereal::PortableBinaryInputArchive& archive) {
            SceneType& typed_scene = std::any_cast<SceneType&>(scene.user_data);
            // archive(typed_scene);
        };

        _scene_type.json_save = [](scene_implementation& scene, cereal::JSONOutputArchive& archive) {
            SceneType& typed_scene = std::any_cast<SceneType&>(scene.user_data);
            // archive(typed_scene);
        };

        _scene_type.json_load = [](scene_implementation& scene, cereal::JSONInputArchive& archive) {
            SceneType& typed_scene = std::any_cast<SceneType&>(scene.user_data);
            // archive(typed_scene);
        };

        engine_scene_type_ids()[std::type_index(typeid(SceneType))] = type_id;
        engine_scene_types()[std::move(type_id)] = std::move(_scene_type);
    }

    template <typename... ComponentTypes, typename Callback>
    void each_view(Callback&& callback)
    {
        for (scene_implementation& _scene : engine_scenes()) {
            _scene.components.view<ComponentTypes...>().each(std::forward<Callback>(callback));
        }
    }

    template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
    void each_view(entt::exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
    {
        for (scene_implementation& _scene : engine_scenes()) {
            _scene.components.view<ComponentTypes...>(exclude).each(std::forward<Callback>(callback));
        }
    }

    void update_callbacks();
}

/// @brief
struct scene_context {

    /// @brief Creates a new entity in the scene and returns its handle
    /// @return the new entity handle
    [[nodiscard]] scene_entity emplace_entity();

    /// @brief Schedules an entity for erasure at the end of the current frame. The entity will be removed from the scene and all its components will be freed.
    /// @param entity
    void mark_erase_entity(const scene_entity entity);

    /// @brief Schedules the current scene for erasure at the end of the current frame. The scene will be removed from the engine and all its resources will be freed.
    void mark_erase_self();

    template <typename... ComponentTypes, typename... ExcludeComponentTypes>
    void each_view(const std::function<void(const scene_entity, ComponentTypes&&...)>& callback, entt::exclude_t<ExcludeComponentTypes...> exclude);

    template <typename... ComponentTypes, typename... ExcludeComponentTypes>
    void each_view_self(const std::function<void(const scene_entity, ComponentTypes&&...)>& callback, entt::exclude_t<ExcludeComponentTypes...> exclude);

    //

    animator_component& emplace_animator(const scene_entity entity);
    screen_interface_component& emplace_screen_interface(const scene_entity entity);
    spatial_interface_component& emplace_spatial_interface(const scene_entity entity);
    blockout_model_component& emplace_blockout_model(const scene_entity entity);
    unlit_model_component& emplace_unlit_model(const scene_entity entity);
    passive_rigidbody_component& emplace_passive_rigidbody(const scene_entity entity);
    kinematic_rigidbody_component& emplace_kinematic_rigidbody(const scene_entity entity);
    dynamic_rigidbody_component& emplace_dynamic_rigidbody(const scene_entity entity);
    speaker_component& emplace_speaker(const scene_entity entity);
    transform_component& emplace_transform(const scene_entity entity);

    // animator_component& emplace_animator(const scene_entity entity);
    // get animator
    // erase animator

    template <typename UserComponentType>
    UserComponentType& emplace_user_component(const scene_entity entity);

private:
    detail::scene_implementation* _self_scene = nullptr;
    friend struct game_context;
    friend void detail::update_callbacks();
};

template <typename... ComponentTypes, typename... ExcludeComponentTypes>
void scene_context::each_view(const std::function<void(const scene_entity, ComponentTypes&&...)>& callback, entt::exclude_t<ExcludeComponentTypes...> exclude)
{
    for (detail::scene_implementation& _scene : detail::engine_scenes()) {
        _scene.components.view<ComponentTypes...>(exclude).each(callback);
    }
}

template <typename... ComponentTypes, typename... ExcludeComponentTypes>
void scene_context::each_view_self(const std::function<void(const scene_entity, ComponentTypes&&...)>& callback, entt::exclude_t<ExcludeComponentTypes...> exclude)
{
    _self_scene->components.view<ComponentTypes...>(exclude).each(callback);
}

}