#include <iostream>
#include <random>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/transform.h>

#include <lucaria/core/error.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/ecs/component/animator.hpp>
#include <lucaria/ecs/component/model.hpp>
#include <lucaria/ecs/component/transform.hpp>
#include <lucaria/ecs/system/motion.hpp>

namespace lucaria {
namespace {

#if LUCARIA_GUIZMO
    extern void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);
#endif

    glm::mat4 sample_motion_track(const motion_track& motion_track, float ratio) // on pourrait le mettre dans le motion_track jsp...
    {
        ozz::animation::Float3TrackSamplingJob position_sampler;
        ozz::animation::QuaternionTrackSamplingJob rotation_sampler;
        ozz::math::Transform _ozz_affine_transform;

        position_sampler.track = &motion_track.get_translation_handle();
        position_sampler.result = &_ozz_affine_transform.translation;
        position_sampler.ratio = ratio;
        if (!position_sampler.Run()) {
            LUCARIA_RUNTIME_ERROR("Failed to run vec3 track sampling job")
        }

        rotation_sampler.track = &motion_track.get_rotation_handle();
        rotation_sampler.result = &_ozz_affine_transform.rotation;
        rotation_sampler.ratio = ratio;
        if (!rotation_sampler.Run()) {
            LUCARIA_RUNTIME_ERROR("Failed to run quat track sampling job")
        }

        _ozz_affine_transform.scale = ozz::math::Float3::one();
        ozz::math::Float4x4 _ozz_transform = ozz::math::Float4x4::FromAffine(_ozz_affine_transform);
        return detail::reinterpret(_ozz_transform);
    }
}

namespace detail {

    void motion_system::advance_controllers()
    {
        each_scene([](entt::registry& scene) {
            scene.view<ecs::animator_component>().each([](ecs::animator_component& animator) {
                for (std::pair<const std::string, ecs::animation_controller>& _pair : animator._controllers) {

                    ecs::animation_controller& _controller = _pair.second;
                    if (_controller._is_playing) {
                        _controller._last_time_ratio = _controller._time_ratio;
                        _controller._time_ratio += static_cast<float>(get_time_delta()); // * 0.1f; // c'est bien le temps qu'il faut multiplier par le weight des fadeins
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
        each_scene([](entt::registry& scene) {
            scene.view<ecs::animator_component>().each([](ecs::animator_component& animator) {
                if (animator._skeleton.has_value()) {

                    ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers;
                    for (std::pair<const std::string, fetched_container<animation>>& _pair : animator._animations) {

                        if (_pair.second.has_value()) {

                            const ecs::animation_controller& _controller = animator._controllers[_pair.first];
                            ozz::animation::Animation& _animation = _pair.second.value().get_handle();
                            ozz::vector<ozz::math::SoaTransform>& _local_transforms = animator._local_transforms[_pair.first];
                            ozz::animation::SamplingJob sampling_job;

                            sampling_job.animation = &_animation;
                            sampling_job.context = animator._sampling_context.get();
                            sampling_job.ratio = _controller._time_ratio;
                            sampling_job.output = make_span(_local_transforms);
                            if (!sampling_job.Run()) {
                                LUCARIA_RUNTIME_ERROR("Failed to run animation sampling job")
                            }

                            ozz::animation::BlendingJob::Layer& _blend_layer = _blend_layers.emplace_back();
                            _blend_layer.transform = make_span(_local_transforms);
                            _blend_layer.weight = _controller._computed_weight;
                        }
                    }

                    ozz::animation::Skeleton& _skeleton = animator._skeleton.value().get_handle();
                    ozz::animation::BlendingJob _blending_job;
                    ozz::animation::LocalToModelJob ltm_job;

                    _blending_job.threshold = 0.1f; //
                    _blending_job.additive_layers = {};
                    _blending_job.layers = make_span(_blend_layers);
                    _blending_job.rest_pose = _skeleton.joint_rest_poses();
                    _blending_job.output = make_span(animator._blended_local_transforms);
                    if (!_blending_job.Run()) {
                        LUCARIA_RUNTIME_ERROR("Failed to run blending job")
                    }

                    ltm_job.skeleton = &_skeleton;
                    ltm_job.input = make_span(animator._blended_local_transforms);
                    ltm_job.output = make_span(animator._model_transforms);
                    if (!ltm_job.Run()) {
                        LUCARIA_RUNTIME_ERROR("Failed to run local to model job")
                    }
                }
            });
        });
    }

    void motion_system::apply_motion_tracks()
    {
        each_scene([](entt::registry& scene) {
            scene.view<const ecs::animator_component, ecs::transform_component>().each([](const ecs::animator_component& animator, ecs::transform_component& transform) {
                for (const std::pair<const std::string, fetched_container<motion_track>>& _pair : animator._motion_tracks) {

                    if (_pair.second.has_value()) {

                        const motion_track& _motion_track = _pair.second.value();
                        const ecs::animation_controller& _controller = animator._controllers.at(_pair.first);
                        const glm::mat4 _new_transform = transform._transform * sample_motion_track(_motion_track, _controller._time_ratio);
                        const glm::mat4 _last_transform = transform._transform * sample_motion_track(_motion_track, _controller._last_time_ratio);
                        glm::mat4 _delta_transform = _new_transform * glm::inverse(_last_transform);

                        if (_controller._has_looped) {
                            const glm::mat4 _end_transform = transform._transform * sample_motion_track(_motion_track, 1.f);
                            const glm::mat4 _begin_transform = transform._transform * sample_motion_track(_motion_track, 0.f);
                            _delta_transform = _end_transform * glm::inverse(_begin_transform) * _delta_transform;
                        }

                        // _delta_transform = glm::interpolate(glm::mat4(1.f), _delta_transform, _controller._computed_weight);
                        transform.set_transform_relative(_delta_transform);
                    }
                }
            });
        });
    }

    void motion_system::collect_debug_guizmos()
    {
        // #if LUCARIA_GUIZMO
        each_scene([](entt::registry& scene) {
            scene.view<ecs::animator_component>(entt::exclude<ecs::transform_component>).each([](ecs::animator_component& animator) {
                if (animator._skeleton.has_value()) {

                    const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                    if (!model_transforms.empty()) {
                        const auto& joint_parents = animator._skeleton.value().get_handle().joint_parents();

                        if (model_transforms.size() != joint_parents.size()) {
                            LUCARIA_RUNTIME_ERROR("Mismatch between model transforms and joint parents sizes")
                        }
                        for (size_t i = 0; i < model_transforms.size(); ++i) {
                            int parent_index = joint_parents[i];
                            if (parent_index == ozz::animation::Skeleton::kNoParent) {
                                // // Handle root bone separately
                                continue;
                            }
                            const ozz::math::Float4x4& current_transform = model_transforms[i];
                            const ozz::math::Float4x4& parent_transform = model_transforms[parent_index];
#if defined(__EMSCRIPTEN__)
                            btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                            btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
#else
                            // btVector3 from(parent_transform.cols[3][0], parent_transform.cols[3][1], parent_transform.cols[3][2]);
                            // btVector3 to(current_transform.cols[3][0], current_transform.cols[3][1], current_transform.cols[3][2]);
#endif
                            btVector3 color(1.0f, 0.0f, 0.0f); // Red color for bones
                            // detail::draw_guizmo_line(from, to, color);
                        }
                    }
                }
            });

            scene.view<ecs::animator_component, ecs::transform_component>().each([](ecs::animator_component& animator, ecs::transform_component& transform) {
                if (animator._skeleton.has_value()) {

                    const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                    if (!model_transforms.empty()) {
                        const auto& joint_parents = animator._skeleton.value().get_handle().joint_parents();
                        if (model_transforms.size() != joint_parents.size()) {
                            LUCARIA_RUNTIME_ERROR("Mismatch between model transforms and joint parents sizes")
                        }
                        for (size_t i = 0; i < model_transforms.size(); ++i) {
                            int parent_index = joint_parents[i];
                            if (parent_index == ozz::animation::Skeleton::kNoParent) {
                                // // Handle root bone separately
                                continue;
                            }
                            const ozz::math::Float4x4& _modifier_transform = reinterpret_ozz(transform._transform);
                            const ozz::math::Float4x4 current_transform = _modifier_transform * model_transforms[i];
                            const ozz::math::Float4x4 parent_transform = _modifier_transform * model_transforms[parent_index];
#if defined(__EMSCRIPTEN__)
                            btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                            btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
#else
                            // btVector3 from(parent_transform.cols[3][0], parent_transform.cols[3][1], parent_transform.cols[3][2]);
                            // btVector3 to(current_transform.cols[3][0], current_transform.cols[3][1], current_transform.cols[3][2]);
#endif
                            btVector3 color(1.0f, 0.0f, 0.0f); // Red color for bones
                            // detail::draw_guizmo_line(from, to, color);
                        }
                    }
                }
            });
        });
        // #endif
    }

}
}
