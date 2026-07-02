#include <algorithm>

#include <lucaria/core/manager_game.hpp>
#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/core/serialize_scenes.hpp>
#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_interface.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_speaker.hpp>
#include <lucaria/engine/component_transform.hpp>
#include <lucaria/engine/context_game.hpp>

namespace lucaria {
namespace detail {

    namespace {

        template <typename ComponentType>
        [[nodiscard]] std::vector<snapshot_object_scene_component<ComponentType>> _save_component_group(
            storage_registry& registry,
            object_entity_scene_index scene,
            const std::unordered_map<object_entity, uint32>& entity_ids)
        {
            std::vector<snapshot_object_scene_component<ComponentType>> components = {};

            registry.view<ComponentType>(scene, exclude<>).each([&](object_entity entity, ComponentType& component) {
                snapshot_object_scene_component<ComponentType>& back = components.emplace_back();
                back.component_save_id = entity_ids.at(entity);
                back.component_save = &component;
            });

            return components;
        }

    }

    void manager_scenes::start_scene(context_game& game, object_user_scene& scene)
    {
        user_scene_type_callbacks& callbacks = scene_types.at(scene.type_id);

        if (callbacks.start) {
            // index_for_context is set by manager_scenes::construct
            callbacks.start(game, scene);
        }
    }

    void manager_scenes::clear_runtime_for_reload(context_game& game)
    {
        for (object_user_scene& scene : scenes) {
            auto it = scene_types.find(scene.type_id);
            if (it != scene_types.end() && it->second.stop) {
                it->second.stop(game, scene);
            }
        }

        scenes.clear();
        registry.clear_runtime();
        index_for_context = 0;
    }

    void manager_scenes::clear_plugin_registrations_for_reload()
    {
#if !defined(LUCARIA_DISABLE_COMPUTE_SPIRV)
        compiler.reset();
#endif
        gsl_systems.clear();
        scene_types.clear();
        scene_type_ids.clear();
        user_component_types.clear();
        user_component_type_ids.clear();
    }

    void manager_scenes::update_callbacks(context_game& game)
    {
        for (std::size_t index = 0; index < scenes.size(); ++index) {
            object_user_scene& scene = scenes[index];

            user_scene_type_callbacks& callbacks = scene_types.at(scene.type_id);

            if (callbacks.update) {
                index_for_context = static_cast<uint16>(index);
                scene.index_for_context = index_for_context;
                callbacks.update(game, scene);
            }
        }
    }

    void manager_scenes::update_systems(manager_game& game)
    {
        game.rendering.update_apply_camera_rotation(game.scenes);
        game.motion.update_advance_controllers(game.window, game.scenes);
        game.motion.update_apply_animations(game.scenes);
        game.motion.update_apply_motion_tracks(game.window, game.scenes);
        game.dynamics.update_step_simulation(game.window, game.scenes);
        game.dynamics.update_compute_collisions(game.scenes);
        game.mixer.update_apply_speaker_transforms(game.scenes);
        game.mixer.update_apply_listener_transform(game.scenes);
        game.motion.update_collect_debug_guizmos(game.rendering, game.scenes);
        game.dynamics.update_collect_debug_guizmos(game.rendering, game.scenes);
        game.rendering.update_clear_screen(game.window, game.scenes);
        game.rendering.update_compute_projection(game.window, game.scenes);
        game.rendering.update_compute_view_projection(game.scenes);
        game.rendering.update_draw_skybox(game.scenes);
        game.rendering.update_draw_blockout_meshes(game.scenes);
        game.rendering.update_draw_unlit_meshes(game.scenes);
        game.rendering.update_draw_unlit_skinned_meshes(game.scenes);
        game.rendering.update_draw_imgui_spatial_interfaces(game.window, game.input, game.scenes);
        game.rendering.update_draw_post_processing(game.window, game.scenes);
        game.rendering.update_draw_imgui_screen_interfaces(game.window, game.scenes);
        game.rendering.update_draw_debug_guizmos(game.dynamics, game.input, game.scenes);
    }

    snapshot_manager_scene make_snapshot(manager_scenes& manager, mappings_manager_scene_save& mappings)
    {
        snapshot_manager_scene snapshot;

        mappings.saving_scene_manager = &manager;
        mappings.save_map_scene_ids.clear();
        mappings.save_map_scene_entities.clear();

        uint32 next_scene_id = 1;

        for (object_user_scene& scene : manager.scenes) {
            const object_entity_scene_index segment = scene.index_for_context;

            mappings.save_map_scene_ids.emplace(segment, next_scene_id++);
            auto& entity_ids = mappings.save_map_scene_entities[segment];

            uint32 next_entity_id = 1;

            if (segment >= manager.registry.scene_allocators.size()) {
                continue;
            }

            const auto& allocator = manager.registry.scene_allocators[segment];

            std::vector<bool> is_free(
                static_cast<std::size_t>(allocator.next_local),
                false);

            for (const object_entity_local_index local : allocator.free_list) {
                if (local < is_free.size()) {
                    is_free[local] = true;
                }
            }

            for (
                object_entity_local_index local = 0;
                local < allocator.next_local;
                ++local) {
                if (is_free[local]) {
                    continue;
                }

                const object_entity entity = make_entity(
                    segment,
                    local,
                    allocator.generations[local]);

                if (!manager.registry.valid(entity)) {
                    continue;
                }

                entity_ids.emplace(entity, next_entity_id++);
            }
        }

        for (object_user_scene& scene : manager.scenes) {
            const object_entity_scene_index segment = scene.index_for_context;
            const std::unordered_map<object_entity, uint32>& entity_ids = mappings.save_map_scene_entities.at(segment);

            snapshot_object_scene& saved = snapshot.scenes.emplace_back();

            saved.scene_save_id = mappings.save_map_scene_ids.at(segment);

            for (const auto& [entity, entity_id] : entity_ids) {
                saved.components.entity_save_ids.push_back(entity_id);
            }

            saved.type_id = scene.type_id;
            saved.scene = &scene;
            saved.scene_type_callbacks = &manager.scene_types.at(scene.type_id);

            saved.components.animators = _save_component_group<component_animator>(manager.registry, segment, entity_ids);
            saved.components.screen_interfaces = _save_component_group<component_interface_screen>(manager.registry, segment, entity_ids);
            saved.components.spatial_interfaces = _save_component_group<component_interface_spatial>(manager.registry, segment, entity_ids);
            saved.components.blockout_models = _save_component_group<component_model_blockout>(manager.registry, segment, entity_ids);
            saved.components.unlit_models = _save_component_group<component_model_unlit>(manager.registry, segment, entity_ids);
            saved.components.passive_rigidbodies = _save_component_group<component_rigidbody_passive>(manager.registry, segment, entity_ids);
            saved.components.kinematic_rigidbodies = _save_component_group<component_rigidbody_kinematic>(manager.registry, segment, entity_ids);
            saved.components.dynamic_rigidbodies = _save_component_group<component_rigidbody_dynamic>(manager.registry, segment, entity_ids);
            saved.components.speakers = _save_component_group<component_speaker_spatial>(manager.registry, segment, entity_ids);
            saved.components.transforms = _save_component_group<component_transform>(manager.registry, segment, entity_ids);

            for (auto& [type_id, callbacks] : manager.user_component_types) {
                snapshot_object_scene_user_component_group& user_component = saved.user_components.emplace_back();

                user_component.type_id = type_id;
                user_component.scene = &scene;
                user_component.component_type_callbacks = &callbacks;
            }
        }

        return snapshot;
    }

}
}

namespace lucaria {
namespace detail {

    void manager_scenes::save(context_save_game& context)
    {
        snapshot_manager_scene snapshot = make_snapshot(*this, context.mappings.scenes);
        context.field("scenes", snapshot.scenes);
    }

    void manager_scenes::load(context_load_game& context)
    {
        std::vector<snapshot_object_scene> loaded_scenes = {};
        context.field("scenes", loaded_scenes);
    }

}
}
