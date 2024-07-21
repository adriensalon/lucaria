#include <iostream>

#include <btBulletDynamicsCommon.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <glm/gtc/type_ptr.hpp>

#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/motion.hpp>
#include <core/world.hpp>
#include <core/window.hpp>

namespace detail {

#if LUCARIA_GUIZMO
extern void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);

void draw_guizmo_cone(const btVector3& from, const btVector3& to, const btVector3& color) 
{
    draw_guizmo_line(from, to, color);
}

#endif

glm::mat4 ozz_to_glm(const ozz::math::Float4x4& matrix) 
{
    glm::mat4 _matrix;
    std::memcpy(glm::value_ptr(_matrix), matrix.cols, sizeof(matrix.cols));
    return _matrix;
}

}

void motion_system::blend_animations()
{
    float _time_delta = get_time_delta();
    each_level([](entt::registry& registry) {
        registry.view<animator_component>().each([](animator_component& animator) {
            if (animator._skeleton.has_value()) {
                ozz::animation::Skeleton& _skeleton = animator._skeleton.value();
                ozz::vector<ozz::math::Float4x4>& _model_transforms = animator._model_transforms;
                for (std::pair<const glm::uint, fetch_container<animation_ref>>& _pair : animator._animations) {
                    if (_pair.second.has_value()) {
                        animation_controller& _controller = animator._controllers[_pair.first];
                        ozz::animation::Animation& _animation = _pair.second.value();
                        ozz::vector<ozz::math::SoaTransform>& _local_transforms = animator._local_transforms[_pair.first];

                        // update controller with delta_time

                        // _controller.time_ratio += 0.01f;

                        ozz::animation::SamplingJob sampling_job;
                        sampling_job.animation = &_animation;
                        sampling_job.context = animator._sampling_context.get();
                        sampling_job.ratio = _controller.time_ratio;
                        sampling_job.output = make_span(_local_transforms);
                        if (!sampling_job.Run()) {
#if LUCARIA_DEBUG
                            std::cout << "Impossible to run sampling job." << std::endl;
                            std::terminate();
#endif
                        }

                        // ca faudra bouger
                        ozz::animation::LocalToModelJob ltm_job;
                        ltm_job.skeleton = &_skeleton;
                        ltm_job.input = make_span(_local_transforms);
                        ltm_job.output = make_span(_model_transforms);
                        if (!ltm_job.Run()) {
#if LUCARIA_DEBUG
                            std::cout << "Impossible to run local to model job." << std::endl;
                            std::terminate();
#endif
                        }
                    }
                }
            }
        });
    });
}

void motion_system::apply_root_motion()
{
    each_level([](entt::registry& registry) {
        registry.view<animator_component, transform_component>().each([](animator_component& animator, transform_component& transform) {
            if (animator._motion_bone_index.has_value()) {
                const glm::uint _motion_bone_index = animator._motion_bone_index.value();
                ozz::math::Float4x4& _ozz_motion_transform = animator._model_transforms[_motion_bone_index];
                const glm::mat4 _motion_transform = detail::ozz_to_glm(_ozz_motion_transform);
                // const ozz::math::Float4x4 _ozz_inverse_motion_transform = ozz::math::Invert(_ozz_motion_transform);
                // _ozz_motion_transform = ozz::math::Float4x4::identity();
                // for (glm::uint _index = 0; _index < animator._model_transforms.size() && _index != _motion_bone_index; ++_index) {
                //     ozz::math::Float4x4& _transform = animator._model_transforms[_index];
                //     _transform = _ozz_inverse_motion_transform * _transform;
                //     std::cout << "inverting tf" << std::endl;
                // }
                // transform.transform_relative(_motion_transform);
            }
        });
    });
}

void motion_system::apply_foot_ik()
{
    // animators, rigidbodies (stores ground_normal)
}

void motion_system::skin_meshes()
{
    each_level([](entt::registry& registry) {
        registry.view<animator_component, blockout_model_component>().each([](animator_component& animator, blockout_model_component& model) {
            // std::vector<glm::vec3> _positions;
            // // todo
            // model._mesh.value().update_positions(_positions);
        });
    });
}

void motion_system::collect_debug_guizmos()
{
#if LUCARIA_GUIZMO
    each_level([](entt::registry& registry) {
        registry.view<animator_component>().each([](animator_component& animator) {
            if (animator._skeleton.has_value()) {
                // add cones from animator._model_transforms with detail::draw_line(from, to, color)

                skeleton_ref& skeleton = animator._skeleton.value();
                const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                // Ensure the skeleton and model transforms are valid
                if (!model_transforms.empty()) {
                    const auto& joint_parents = skeleton.joint_parents();

                    // Iterate through each joint to draw cones
                    for (size_t i = 0; i < model_transforms.size(); ++i) {
                        int parent_index = joint_parents[i];
                        if (parent_index == ozz::animation::Skeleton::kNoParent) {
                            continue;  // Skip root joints
                        }

                        // Get the current and parent joint transforms
                        const auto& current_transform = model_transforms[i];
                        const auto& parent_transform = model_transforms[parent_index];

                        // Convert Ozz transforms to Bullet vectors
                        btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                        btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
                        btVector3 color(1.0f, 0.0f, 0.0f);  // Red color for bones

                        // Draw the cone
                        detail::draw_guizmo_cone(from, to, color);
                    }
                }
            }
        });
    });
#endif
}