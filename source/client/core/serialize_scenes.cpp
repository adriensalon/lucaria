#include <lucaria/core/manager_game.hpp>
#include <lucaria/core/manager_scenes.hpp>
#include <lucaria/public/component_animator.hpp>
#include <lucaria/public/component_interface.hpp>
#include <lucaria/public/component_model.hpp>
#include <lucaria/public/component_rigidbody.hpp>
#include <lucaria/public/component_speaker.hpp>
#include <lucaria/public/component_transform.hpp>
#include <lucaria/public/context_game.hpp>

namespace lucaria {
namespace detail {

    namespace {

        template <typename ComponentType>
        [[nodiscard]] std::vector<recipe_object_scene_component<ComponentType>> _save_component_group(entt::registry* registry, const std::unordered_map<entt::entity, uint32>& entity_ids)
        {
            std::vector<recipe_object_scene_component<ComponentType>> _components = {};
            registry->view<ComponentType>().each(
                [&](entt::entity entity, ComponentType& component) {
                    recipe_object_scene_component<ComponentType>& _back = _components.emplace_back();
                    _back.component_save_id = entity_ids.at(entity);
                    _back.component_save = &component;
                    // _back.component = std::make_shared<ComponentType>(component);
                });
            return _components;
        }
    }

    void manager_scenes::start_scene(context_game& game, object_user_scene& scene)
    {
        user_scene_type_callbacks& _callbacks = scene_types.at(scene.type_id);
        if (_callbacks.start) {
            current_scene = &scene;
            _callbacks.start(game, scene);
        }
    }

    void manager_scenes::update_callbacks(context_game& game)
    {
        for (std::unique_ptr<object_user_scene>& _scene : scenes) {
            user_scene_type_callbacks& _callbacks = scene_types.at(_scene->type_id);
            if (_callbacks.update) {
                current_scene = _scene.get();
                _callbacks.update(game, *_scene.get());
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

    void apply_recipes_for(
        manager_window& window,
        manager_assets& objects,
        container_cache_vector<object_font>& cached_vector,
        mappings_container_cache_vector_load<object_font>& mappings,
        std::vector<recipe_object_entry<recipe_object_font>>& recipes)
    {
        for (auto& entry : recipes) {
            container_cache<object_font>* cell = apply_recipe(window, objects, cached_vector, entry.recipe);
            if (cell == nullptr) {
                LUCARIA_DEBUG_ERROR("Failed to apply font recipe");
                continue;
            }
            mappings.set(entry.save_id, cell);
        }
    }

    recipe_manager_scene make_recipe(manager_scenes& manager, mappings_manager_scene_save& mappings)
    {
        recipe_manager_scene recipe;

        mappings.save_map_scene_ids.clear();
        mappings.save_map_scene_entities.clear();

        uint32 next_scene_id = 1;

        for (std::unique_ptr<object_user_scene>& scene : manager.scenes) {
            entt::registry* registry = &scene->components;
            mappings.save_map_scene_ids.emplace(registry, next_scene_id++);
            auto& entity_ids = mappings.save_map_scene_entities[registry];

            uint32 next_entity_id = 1;
            for (entt::entity entity : registry->storage<entt::entity>()) {
                entity_ids.emplace(entity, next_entity_id++);
            }
        }

        for (std::unique_ptr<object_user_scene>& scene : manager.scenes) {
            entt::registry* registry = &scene->components;
            const std::unordered_map<entt::entity, lucaria::uint32>& entity_ids = mappings.save_map_scene_entities.at(registry);

            recipe_object_scene& saved = recipe.scenes.emplace_back();
            saved.scene_save_id = mappings.save_map_scene_ids.at(registry);
            for (const auto& [entity, entity_id] : entity_ids) {
                saved.components.entity_save_ids.push_back(entity_id);
            }

            saved.type_id = scene->type_id;
            saved.scene = scene.get();
            saved.scene_type_callbacks = &manager.scene_types.at(scene->type_id);

            saved.components.animators = _save_component_group<component_animator>(registry, entity_ids);
            saved.components.screen_interfaces = _save_component_group<component_interface_screen>(registry, entity_ids);
            saved.components.spatial_interfaces = _save_component_group<component_interface_spatial>(registry, entity_ids);
            saved.components.blockout_models = _save_component_group<component_model_blockout>(registry, entity_ids);
            saved.components.unlit_models = _save_component_group<component_model_unlit>(registry, entity_ids);
            saved.components.passive_rigidbodies = _save_component_group<component_rigidbody_passive>(registry, entity_ids);
            saved.components.kinematic_rigidbodies = _save_component_group<component_rigidbody_kinematic>(registry, entity_ids);
            saved.components.dynamic_rigidbodies = _save_component_group<component_rigidbody_dynamic>(registry, entity_ids);
            saved.components.speakers = _save_component_group<component_speaker_spatial>(registry, entity_ids);
            saved.components.transforms = _save_component_group<component_transform>(registry, entity_ids);

            for (auto& [type_id, callbacks] : manager.user_component_types) {
                recipe_object_scene_user_component_group& user_component = saved.user_components.emplace_back();
                user_component.type_id = type_id;
                user_component.scene = scene.get();
                user_component.component_type_callbacks = &callbacks;
            }
        }

        return recipe;
    }



}
}