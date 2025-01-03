#include <iostream>
#include <random>

#include <btBulletDynamicsCommon.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/base/maths/transform.h>

#include <core/math.hpp>
#include <core/window.hpp>
#include <core/world.hpp>
#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/motion.hpp>

namespace detail {

void print_matrix(const ozz::math::Float4x4& matrix)
{
    // const ozz::math::SimdFloat4 cols[] = matrix.cols;

    // Print the matrix in a 4x4 grid
    std::cout << std::endl;
    std::cout << "[ " << matrix.cols[0].x << " " << matrix.cols[1].x << " " << matrix.cols[2].x << " " << matrix.cols[3].x << " ]" << std::endl;
    std::cout << "[ " << matrix.cols[0].y << " " << matrix.cols[1].y << " " << matrix.cols[2].y << " " << matrix.cols[3].y << " ]" << std::endl;
    std::cout << "[ " << matrix.cols[0].z << " " << matrix.cols[1].z << " " << matrix.cols[2].z << " " << matrix.cols[3].z << " ]" << std::endl;
    std::cout << "[ " << matrix.cols[0].w << " " << matrix.cols[1].w << " " << matrix.cols[2].w << " " << matrix.cols[3].w << " ]" << std::endl;
}

void print_matrix(const glm::mat4& matrix)
{
    print_matrix(reinterpret_ozz(matrix));
}

#if LUCARIA_GUIZMO
extern void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);
#endif

glm::mat4 sample_motion_track(const motion_track_ref& motion_track, float ratio) // on pourrait le mettre dans le motion_track_ref jsp...
{
    glm::mat4 _transform;
    ozz::animation::Float3TrackSamplingJob position_sampler;
    ozz::animation::QuaternionTrackSamplingJob rotation_sampler;
    ozz::math::Transform _ozz_affine_transform;
    ozz::math::Float4x4& _ozz_transform = reinterpret_ozz(_transform);
    position_sampler.track = &(motion_track.first);
    position_sampler.result = &(_ozz_affine_transform.translation);
    position_sampler.ratio = ratio;
    if (!position_sampler.Run()) {
#if LUCARIA_DEBUG
        std::cout << "Impossible to run vec3 track sampling job." << std::endl;
        std::terminate();
#endif
    }
    rotation_sampler.track = &(motion_track.second);
    rotation_sampler.result = &(_ozz_affine_transform.rotation);
    rotation_sampler.ratio = ratio;
    if (!rotation_sampler.Run()) {
#if LUCARIA_DEBUG
        std::cout << "Impossible to run quat track sampling job." << std::endl;
        std::terminate();
#endif
    }
    _ozz_affine_transform.scale = ozz::math::Float3::one();
    _ozz_transform = ozz::math::Float4x4::FromAffine(_ozz_affine_transform);
    return _transform;
}

}

void motion_system::advance_controllers()
{
    each_scene([](scene_data& scene) {
        scene.components.view<animator_component>().each([](animator_component& animator) {
            for (std::pair<const unsigned int, animation_controller>& _pair : animator._controllers) {
                animation_controller& _controller = animator._controllers[_pair.first];
                if (_controller._is_playing) {
                    _controller._last_time_ratio = _controller._time_ratio;
                    _controller._time_ratio += get_time_delta();// * 0.1f; // c'est bien le temps qu'il faut multiplier par le weight des fadeins
                }
                _controller._has_looped = _controller._time_ratio > 1.f;
                _controller._time_ratio = glm::mod(_controller._time_ratio, 1.f);
                _controller._computed_weight = 1.f;
                // _controller._computed_weight = _controller._weight; // add fade in and fade out
            }
        });
    });
}

void motion_system::apply_animations()
{
    each_scene([](scene_data& scene) {
        scene.components.view<animator_component>().each([](animator_component& animator) {
            if (animator._skeleton.has_value()) {
                ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers; 
                for (std::pair<const unsigned int, fetch_container<animation_ref>>& _pair : animator._animations) {
                    if (_pair.second.has_value()) {
                        const animation_controller& _controller = animator._controllers[_pair.first];
                        ozz::animation::Animation& _animation = _pair.second.value();
                        ozz::vector<ozz::math::SoaTransform>& _local_transforms = animator._local_transforms[_pair.first];
                        ozz::animation::SamplingJob sampling_job;
                        sampling_job.animation = &_animation;
                        sampling_job.context = animator._sampling_context.get();
                        sampling_job.ratio = _controller._time_ratio;
                        sampling_job.output = make_span(_local_transforms);
                        if (!sampling_job.Run()) {
#if LUCARIA_DEBUG
                            std::cout << "Impossible to run sampling job." << std::endl;
                            std::terminate();
#endif
                        }
                        ozz::animation::BlendingJob::Layer& _blend_layer = _blend_layers.emplace_back();
                        _blend_layer.transform = make_span(_local_transforms);
                        _blend_layer.weight = _controller._computed_weight;
                    }
                }
                ozz::animation::Skeleton& _skeleton = animator._skeleton.value();
                ozz::animation::BlendingJob _blending_job;
                ozz::animation::LocalToModelJob ltm_job;
                _blending_job.threshold = 0.1f; //
                _blending_job.additive_layers = {};
                _blending_job.layers = make_span(_blend_layers);
                _blending_job.rest_pose = _skeleton.joint_rest_poses();
                _blending_job.output = make_span(animator._blended_local_transforms);
                if (!_blending_job.Run()) {
#if LUCARIA_DEBUG
                    std::cout << "Impossible to run blending job." << std::endl;
                    std::terminate();
#endif
                }
                ltm_job.skeleton = &_skeleton;
                ltm_job.input = make_span(animator._blended_local_transforms);
                ltm_job.output = make_span(animator._model_transforms);
                if (!ltm_job.Run()) {
#if LUCARIA_DEBUG
                    std::cout << "Impossible to run local to model job." << std::endl;
                    std::terminate();
#endif
                }
            }
        });
    });
}

void motion_system::apply_motion_tracks()
{
    each_scene([](scene_data& scene) {
        scene.components.view<const animator_component, transform_component>().each([](const animator_component& animator, transform_component& transform) {
            for (const std::pair<const unsigned int, fetch_container<motion_track_ref>>& _pair : animator._motion_tracks) {
                if (_pair.second.has_value()) {
                    const motion_track_ref& _motion_track = _pair.second.value();
                    const animation_controller& _controller = animator._controllers.at(_pair.first);
                    const glm::mat4 _new_transform = transform._transform * detail::sample_motion_track(_motion_track, _controller._time_ratio);
                    const glm::mat4 _last_transform = transform._transform * detail::sample_motion_track(_motion_track, _controller._last_time_ratio);
                    glm::mat4 _delta_transform = _new_transform * glm::inverse(_last_transform);
                    if (_controller._has_looped) {
                        const glm::mat4 _end_transform = transform._transform * detail::sample_motion_track(_motion_track, 1.f);
                        const glm::mat4 _begin_transform = transform._transform * detail::sample_motion_track(_motion_track, 0.f);
                        _delta_transform = _end_transform * glm::inverse(_begin_transform) * _delta_transform;
                    }
                    // _delta_transform = glm::interpolate(glm::mat4(1.f), _delta_transform, _controller._computed_weight);
                    transform.transform_relative(_delta_transform);
                }
            }
        });
    });
}

void motion_system::collect_debug_guizmos()
{
#if LUCARIA_GUIZMO
    each_scene([](scene_data& scene) {
        scene.components.view<animator_component>(entt::exclude<transform_component>).each([](animator_component& animator) {
            if (animator._skeleton.has_value()) {
                const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                if (!model_transforms.empty()) {
                    const auto& joint_parents = animator._skeleton.value().joint_parents();
#if LUCARIA_DEBUG
                    if (model_transforms.size() != joint_parents.size()) {
                        std::cout << "Mismatch between model transforms and joint parents sizes." << std::endl;
                        std::terminate();
                    }
#endif
                    for (size_t i = 0; i < model_transforms.size(); ++i) {
                        int parent_index = joint_parents[i];
                        if (parent_index == ozz::animation::Skeleton::kNoParent) {
                            // // Handle root bone separately
                            continue;
                        }
                        const ozz::math::Float4x4& current_transform = model_transforms[i];
                        const ozz::math::Float4x4& parent_transform = model_transforms[parent_index];
                        btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                        btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
                        btVector3 color(1.0f, 0.0f, 0.0f); // Red color for bones
                        detail::draw_guizmo_line(from, to, color);
                    }
                }
            }
        });
        scene.components.view<animator_component, transform_component>().each([](animator_component& animator, transform_component& transform) {
            if (animator._skeleton.has_value()) {
                const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                if (!model_transforms.empty()) {
                    const auto& joint_parents = animator._skeleton.value().joint_parents();
#if LUCARIA_DEBUG
                    if (model_transforms.size() != joint_parents.size()) {
                        std::cout << "Mismatch between model transforms and joint parents sizes." << std::endl;
                        std::terminate();
                    }
#endif
                    for (size_t i = 0; i < model_transforms.size(); ++i) {
                        int parent_index = joint_parents[i];
                        if (parent_index == ozz::animation::Skeleton::kNoParent) {
                            // // Handle root bone separately
                            continue;
                        }
                        const ozz::math::Float4x4& _modifier_transform = reinterpret_ozz(transform._transform);
                        const ozz::math::Float4x4 current_transform = _modifier_transform * model_transforms[i];
                        const ozz::math::Float4x4 parent_transform = _modifier_transform * model_transforms[parent_index];
                        btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                        btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
                        btVector3 color(1.0f, 0.0f, 0.0f); // Red color for bones
                        detail::draw_guizmo_line(from, to, color);
                    }
                }
            }
        });
    });
#endif
}