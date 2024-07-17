#include <iostream>

#include <ozz/base/maths/soa_transform.h>

#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/system/motion.hpp>
#include <core/world.hpp>

namespace detail {

// void soa_lerp(const ozz::math::SoaTransform& source, ozz::math::SoaTransform& destination, const glm::float32 weight)
// {
//     ozz::math::SimdFloat4 _simd_weight = ozz::math::simd_float4::Load1(weight);
//     destination.translation = ozz::math::Lerp(destination.translation, source.translation, _simd_weight);
//     destination.rotation = ozz::math::Lerp(destination.rotation, source.rotation, _simd_weight);
//     destination.scale = ozz::math::Lerp(destination.scale, source.scale, _simd_weight);
// }

// bool is_ready_for_binding(std::optional<mesh_ref>& mesh, std::optional<skeleton_ref>& skeleton)
// {
//     const bool _mesh_has_value = mesh.has_value();
//     const bool _skeleton_has_value = skeleton.has_value();
//     const bool _positions_not_empty = _mesh_has_value && _skeleton_has_value && !mesh.value().get_positions().empty();
//     return _positions_not_empty;
// }

// void prepare_sampling(animation_sampler_ref& animation_sampler, skeleton_ref& skeleton, animation_ref& animation)
// {
//     animation_sampler.local_transforms.resize(skeleton.get_skeleton().num_soa_joints());
//     // animation_sampler.sampling_job.animation = &animation.get_animation();
//     // // animation_sampler.sampling_job.context.
//     // animation_sampler.sampling_job.context->Resize(animation.get_animation().num_tracks());
//     // animation_sampler.sampling_job.output = ozz::make_span(animation_sampler.local_transforms);
//     // animation_sampler.sampling_job.ratio = animation_sampler.ratio;
// }

// void compute_sampling(animation_sampler_ref& animation_sampler, std::vector<ozz::math::SoaTransform>& blended_local_transforms)
// {
//     if (animation_sampler.is_playing) {
//         animation_sampler.sampling_job.ratio = animation_sampler.ratio;
//         if (!animation_sampler.sampling_job.Run()) {
//             animation_sampler.sampling_job.Validate()
// #if LUCARIA_DEBUG
//             std::cout << "Impossible to run sampling job" << std::endl;
//             std::terminate();
// #else
//             return;
// #endif
//         }
//         for (std::size_t _index = 0; _index < animation_sampler.local_transforms.size(); ++_index) {
//             soa_lerp(animation_sampler.local_transforms[_index], blended_local_transforms[_index], animation_sampler.weight);
//         }
//     }
// }

// void prepare_skinning(std::vector<ozz::math::SoaTransform>& blended_local_transforms, std::vector<ozz::math::Float4x4>& model_transforms, ozz::animation::LocalToModelJob& job, skeleton_ref& skeleton)
// {
//     blended_local_transforms.resize(skeleton.get_skeleton().num_soa_joints());
//     model_transforms.resize(skeleton.get_skeleton().num_joints());
//     job.skeleton = &skeleton.get_skeleton();
//     job.input = ozz::make_span(blended_local_transforms);
//     job.output = ozz::make_span(model_transforms);
//     std::cout << blended_local_transforms.size() << " " << model_transforms.size() << std::endl;
// }

// void compute_skinning(std::vector<ozz::math::Float4x4>& model_transforms, ozz::animation::LocalToModelJob& job, mesh_ref& mesh)
// {
//     if (!job.Run()) {
// #if LUCARIA_DEBUG
//             std::cout << "Impossible to run skinning job" << std::endl;
//             std::terminate();
// #else
//             return;
// #endif
//         }
//     mesh.update_skinned_positions([&model_transforms, &job, &mesh](std::vector<glm::vec3>& _computed_positions) {
//         const std::vector<glm::vec3>& tpose_positions = mesh.get_positions();
//         const std::vector<glm::uvec4>& bones = mesh.get_bones();
//         const std::vector<glm::vec4>& weights = mesh.get_weights();
//         for (std::size_t _vertex_index = 0; _vertex_index < tpose_positions.size(); ++_vertex_index) {
//             glm::vec4 skinned_position(0.0f);
//             const glm::vec4 tpose_position(tpose_positions[_vertex_index], 1.0f);
//             for (std::size_t _vertex_bone_index = 0; _vertex_bone_index < 4; ++_vertex_bone_index) {
//                 const glm::uint bone_index = bones[_vertex_index][_vertex_bone_index];
//                 const glm::float32 weight = weights[_vertex_index][_vertex_bone_index];
//                 if (weight > 0.f) {
//                     const glm::mat4 glm_bone_transform = reinterpret_cast<const glm::mat4&>(model_transforms[bone_index]);
//                     const glm::vec4 transformed_position = glm_bone_transform * tpose_position;
//                     skinned_position += weight * transformed_position;
//                 }
//             }
//             _computed_positions[_vertex_index] = glm::vec3(skinned_position);
//         }
//     });
// }

}

// void motion_system::update()
// {
//     each_level([](entt::registry& _registry) {
//         _registry.view<model_component<model_shader::unlit>, animator_component>().each([](model_component<model_shader::unlit>& _model, animator_component& _animator) {
            // if (!_animator._is_bound_to_model && detail::is_ready_for_binding(_model._mesh, _animator._skeleton)) {
            //     _animator._is_bound_to_model = true;
            // }
            // skeleton_ref& _skeleton = _animator._skeleton.value();
            // mesh_ref& _mesh = _model._mesh.value();
            // if (!_animator._is_prepared) {
            //     detail::prepare_skinning(_animator._blended_local_transforms, _animator._model_transforms, _animator._local_to_model_job, _skeleton);
            //     _animator._is_prepared = true;
            // }
            // for (ozz::math::SoaTransform& _transform : _animator._blended_local_transforms) {
            //     _transform = ozz::math::SoaTransform::identity();
            // }
            // for (std::pair<const std::string, animation_sampler_ref>& _pair : _animator._samplers) {
            //     if (!_pair.second.is_prepared) {
            //         std::optional<animation_ref>& _animation = _animator._animations.at(_pair.first);
            //         if (_animation.has_value()) {
            //             detail::prepare_sampling(_pair.second, _skeleton, _animation.value());
            //             _pair.second.is_prepared = true;
            //         }
            //     }
            //     detail::compute_sampling(_pair.second, _animator._blended_local_transforms);
            // }
            // detail::compute_skinning(_animator._model_transforms, _animator._local_to_model_job, _mesh);
//         });
//     });
// }

void motion_system::blend_animations()
{
    // animators -> model space matrices w extracted root motion
}

void motion_system::apply_root_motion()
{
    // animators, transforms
}

void motion_system::apply_foot_ik()
{
    // animators, rigidbodies (stores ground_normal)
}

void motion_system::skin_meshes()
{
    // animators, models
}