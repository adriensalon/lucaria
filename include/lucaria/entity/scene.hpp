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
std::enable_if_t<std::is_aggregate_v<SceneType>> serialize(Archive& archive, SceneType& object)
{
    pfr::for_each_field_with_name(object, [&](std::string_view name, auto& field) {
        archive(cereal::make_nvp(std::string(name).c_str(), field));
    });
}

namespace lucaria {

struct game_context;

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

    template <typename ComponentType>
    struct component_save_entry {
        uint32 entity = 0;
        ComponentType* component = nullptr;

        template <typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                cereal::make_nvp("entity", entity),
                cereal::make_nvp("component", *component));
        }
    };

    struct scene_save {
        std::string type_id;
        std::vector<component_save_entry<transform_component>> transforms;
        std::vector<component_save_entry<unlit_model_component>> unlit_models;
        std::vector<component_save_entry<speaker_component>> speakers;
        std::vector<component_save_entry<dynamic_rigidbody_component>> dynamic_rigidbodies;
        scene_implementation* scene = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));
            // cereal::make_nvp("transforms", transforms),
            archive(cereal::make_nvp("unlit_models", unlit_models));
            // cereal::make_nvp("speakers", speakers),
            // cereal::make_nvp("dynamic_rigidbodies", dynamic_rigidbodies)

            engine_scene_types().at(type_id).json_save(*scene, archive);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("type_id", type_id));
            // cereal::make_nvp("transforms", transforms),
            archive(cereal::make_nvp("unlit_models", unlit_models));
            // cereal::make_nvp("speakers", speakers),
            // cereal::make_nvp("dynamic_rigidbodies", dynamic_rigidbodies)

            engine_scene_types().at(type_id).json_load(*scene, archive);
        }
    };

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

    void update_callbacks();

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
}

/// @brief
struct scene_entity {

    /// @brief
    [[nodiscard]] bool has_value() const;

private:
    entt::entity _entity = entt::null;
    entt::registry* _registry = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        detail::scene_database& _database = detail::engine_scene_database();
        const uint32 _scene_id = _database.save_database.get_scene_id(_registry);
        const uint32 _entity_id = _database.save_database.get_entity_id(_registry, _entity);
        archive(cereal::make_nvp("scene_save_id", _scene_id));
        archive(cereal::make_nvp("entity_save_id", _entity_id));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
        detail::scene_database& _database = detail::engine_scene_database();
        uint32 _scene_id, _entity_id;
        archive(cereal::make_nvp("scene_save_id", _scene_id));
        archive(cereal::make_nvp("entity_save_id", _entity_id));
        _registry = _database.load_database.get_registry(_scene_id);
        _entity = _database.load_database.get_entity(_scene_id, _entity_id);
    }

    friend struct scene_context;
    friend class cereal::access;
};

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

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam Callback
    /// @param callback
    template <typename... ComponentTypes, typename Callback>
    void each_view(Callback&& callback)
    {
        for (detail::scene_implementation& _scene : detail::engine_scenes()) {
            _scene.components.view<ComponentTypes...>().each(std::forward<Callback>(callback));
        }
    }

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam ...ExcludeComponentTypes
    /// @tparam Callback
    /// @param exclude
    /// @param callback
    template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
    void each_view(entt::exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
    {
        for (detail::scene_implementation& _scene : detail::engine_scenes()) {
            _scene.components.view<ComponentTypes...>(exclude).each(std::forward<Callback>(callback));
        }
    }

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam Callback
    /// @param callback
    template <typename... ComponentTypes, typename Callback>
    void each_view_self(Callback&& callback)
    {
        _self_scene->components.view<ComponentTypes...>().each(std::forward<Callback>(callback));
    }

    /// @brief
    /// @tparam ...ComponentTypes
    /// @tparam ...ExcludeComponentTypes
    /// @tparam Callback
    /// @param exclude
    /// @param callback
    template <typename... ComponentTypes, typename... ExcludeComponentTypes, typename Callback>
    void each_view_self(entt::exclude_t<ExcludeComponentTypes...> exclude, Callback&& callback)
    {
        _self_scene->components.view<ComponentTypes...>(exclude).each(std::forward<Callback>(callback));
    }

    /// @brief
    /// @param entity
    /// @return
    animator_component& emplace_animator(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    screen_interface_component& emplace_screen_interface(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    spatial_interface_component& emplace_spatial_interface(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    blockout_model_component& emplace_blockout_model(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    unlit_model_component& emplace_unlit_model(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    passive_rigidbody_component& emplace_passive_rigidbody(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    kinematic_rigidbody_component& emplace_kinematic_rigidbody(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    dynamic_rigidbody_component& emplace_dynamic_rigidbody(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    speaker_component& emplace_speaker(const scene_entity entity);

    /// @brief
    /// @param entity
    /// @return
    transform_component& emplace_transform(const scene_entity entity);

    // get component
    // erase component

    template <typename UserComponentType>
    UserComponentType& emplace_user_component(const scene_entity entity);

private:
    detail::scene_implementation* _self_scene = nullptr;
    friend struct game_context;
    friend void detail::update_callbacks();
};

}
