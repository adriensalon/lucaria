#include <iostream>
#include <random>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/transform.h>

#include <lucaria/component/animator.hpp>
#include <lucaria/component/model.hpp>
#include <lucaria/component/rigidbody.hpp>
#include <lucaria/component/transform.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/system/motion.hpp>

namespace lucaria {
namespace {

    const glm::vec3 _world_up = glm::vec3(0, 1, 0);
    const glm::vec3 _world_forward = glm::vec3(0, 0, 1);

    [[nodiscard]] glm::mat4 _sample_motion_track(const motion_track& track, const glm::float32 ratio)
    {
        // position sampling
        ozz::math::Transform _ozz_affine_transform;
        ozz::animation::Float3TrackSamplingJob _position_sampler;
        _position_sampler.track = &track.get_translation_handle();
        _position_sampler.result = &_ozz_affine_transform.translation;
        _position_sampler.ratio = ratio;
        if (!_position_sampler.Run()) {
            LUCARIA_RUNTIME_ERROR("Failed to run vec3 track sampling job")
        }

        // rotation sampling
        ozz::animation::QuaternionTrackSamplingJob _rotation_sampler;
        _rotation_sampler.track = &track.get_rotation_handle();
        _rotation_sampler.result = &_ozz_affine_transform.rotation;
        _rotation_sampler.ratio = ratio;
        if (!_rotation_sampler.Run()) {
            LUCARIA_RUNTIME_ERROR("Failed to run quat track sampling job")
        }

        _ozz_affine_transform.scale = ozz::math::Float3::one();
        return detail::convert(ozz::math::Float4x4::FromAffine(_ozz_affine_transform));
    }

    [[nodiscard]] glm::mat4 _compute_motion_delta(const motion_track& track, const glm::float32 time_ratio, const glm::float32 last_time_ratio, const glm::float32 computed_weight, const bool has_looped)
    {
        const glm::mat4 _new_motion_transform = _sample_motion_track(track, time_ratio);
        const glm::mat4 _last_motion_transform = _sample_motion_track(track, last_time_ratio);
        glm::mat4 _delta_motion_transform = _new_motion_transform * glm::inverse(_last_motion_transform);
        if (has_looped) {
            const glm::mat4 _end_motion_transform = _sample_motion_track(track, 1.f);
            const glm::mat4 _begin_motion_transform = _sample_motion_track(track, 0.f);
            _delta_motion_transform = _end_motion_transform * glm::inverse(_begin_motion_transform) * _delta_motion_transform;
        }
        _delta_motion_transform = glm::interpolate(glm::mat4(1.f), _delta_motion_transform, computed_weight);
        return _delta_motion_transform;
    }
}

namespace detail {

#if LUCARIA_GUIZMO
    extern void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);
#endif

    void motion_system::advance_controllers()
    {
        detail::each_scene([](entt::registry& scene) {
            scene.view<animator_component>().each([](animator_component& animator) {
                for (std::pair<const std::string, animation_controller>& _pair : animator._controllers) {

                    // advance time
                    animation_controller& _controller = _pair.second;
                    if (_controller._is_playing) {
                        _controller._last_time_ratio = _controller._time_ratio;
                        _controller._time_ratio += _controller._playback_speed * static_cast<glm::float32>(get_time_delta());
                    }
                    _controller._has_looped = _controller._time_ratio > 1.f;
                    _controller._time_ratio = glm::mod(_controller._time_ratio, 1.f);
                    _controller._computed_weight = _controller._weight; // TODO account for fade in and fade out

                    // fire events
                    if (_controller._is_playing && animator._event_tracks[_pair.first].has_value()) {
                        for (const event_data& _event : animator._event_tracks[_pair.first].value().data.events) {
                            if (_controller._last_time_ratio <= _controller._time_ratio
                                    ? (_controller._last_time_ratio < _event.time_normalized) && (_event.time_normalized <= _controller._time_ratio)
                                    : (_controller._last_time_ratio < _event.time_normalized) || (_event.time_normalized <= _controller._time_ratio)) {

                                if (_controller._event_callbacks.find(_event.name) != _controller._event_callbacks.end()) {
                                    _controller._event_callbacks.find(_event.name)->second();
                                }
                            }
                        }
                    }
                }
            });
        });
    }

    void motion_system::apply_animations()
    {
        detail::each_scene([](entt::registry& scene) {
            scene.view<animator_component>().each([](animator_component& animator) {
                if (animator._skeleton.has_value()) {

                    // sampling
                    ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers;
                    for (std::pair<const std::string, fetched_container<animation>>& _pair : animator._animations) {
                        if (_pair.second.has_value()) {
                            const animation_controller& _controller = animator._controllers[_pair.first];
                            ozz::vector<ozz::math::SoaTransform>& _local_transforms = animator._local_transforms[_pair.first];
                            ozz::animation::SamplingJob sampling_job;
                            sampling_job.animation = &_pair.second.value().get_handle();
                            sampling_job.context = animator._sampling_context.get();
                            sampling_job.ratio = _controller._time_ratio;
                            sampling_job.output = make_span(_local_transforms);
                            if (!sampling_job.Run()) {
                                LUCARIA_RUNTIME_ERROR("Failed to run animation sampling job")
                            }

                            // prepare blend layers
                            ozz::animation::BlendingJob::Layer& _blend_layer = _blend_layers.emplace_back();
                            _blend_layer.transform = make_span(_local_transforms);
                            _blend_layer.weight = _controller._computed_weight;
                        }
                    }

                    // blending
                    ozz::animation::Skeleton& _skeleton = animator._skeleton.value().get_handle();
                    ozz::animation::BlendingJob _blending_job;
                    _blending_job.threshold = 0.1f; // TODO let user set parameter
                    _blending_job.additive_layers = {};
                    _blending_job.layers = make_span(_blend_layers);
                    _blending_job.rest_pose = _skeleton.joint_rest_poses();
                    _blending_job.output = make_span(animator._blended_local_transforms);
                    if (!_blending_job.Run()) {
                        LUCARIA_RUNTIME_ERROR("Failed to run blending job")
                    }

                    // local to model
                    ozz::animation::LocalToModelJob _local_to_model_job;
                    _local_to_model_job.skeleton = &_skeleton;
                    _local_to_model_job.input = make_span(animator._blended_local_transforms);
                    _local_to_model_job.output = make_span(animator._model_transforms);
                    if (!_local_to_model_job.Run()) {
                        LUCARIA_RUNTIME_ERROR("Failed to run local to model job")
                    }
                }
            });
        });
    }

    void motion_system::apply_motion_tracks()
    {
        // apply to transform if no dynamic rigidbody
        detail::each_scene([](entt::registry& scene) {
            scene.view<const animator_component, transform_component>(entt::exclude<character_rigidbody_component>).each([](const animator_component& animator, transform_component& transform) {
                for (const std::pair<const std::string, fetched_container<motion_track>>& _pair : animator._motion_tracks) {
                    if (_pair.second.has_value()) {
                        const animation_controller& _controller = animator._controllers.at(_pair.first);
                        if (_controller._computed_weight > 0.f) {
                            const motion_track& _track = _pair.second.value();
                            const glm::mat4 _delta_transform = _compute_motion_delta(_track, _controller._time_ratio, _controller._last_time_ratio, _controller._computed_weight, _controller._has_looped);
                            transform.set_transform_relative(_delta_transform);
                        }
                    }
                }
            });
        });

        // apply to PD targets if dynamic rigidbody
        detail::each_scene([](entt::registry& scene) {
            scene.view<const animator_component, transform_component, character_rigidbody_component>().each([](const animator_component& animator, transform_component& transform, character_rigidbody_component& character) {
                if (character._shape.has_value()) {
                    btRigidBody* _bullet_rigidbody = character._rigidbody.get();
                    const glm::float32 _delta_time = static_cast<glm::float32>(get_time_delta());
                    const glm::vec3 _current_position = transform.get_position();
                    const glm::quat _current_rotation = transform.get_rotation();
                    const glm::mat4 _transform_matrix = character._shape.value().get_feet_to_center() * transform._transform;
                    const btTransform _bullet_transform = convert_bullet(_transform_matrix);
                    _bullet_rigidbody->setWorldTransform(_bullet_transform);

                    // reset on teleporting
                    if (character._last_position != _current_position) {
                        _bullet_rigidbody->getMotionState()->setWorldTransform(_bullet_transform);
                        _bullet_rigidbody->setInterpolationWorldTransform(_bullet_transform);
                        _bullet_rigidbody->setLinearVelocity(btVector3(0, 0, 0));
                        _bullet_rigidbody->setAngularVelocity(btVector3(0, 0, 0));
                        _bullet_rigidbody->clearForces();
                        _bullet_rigidbody->activate(true);
                        character._target_linear_position = _current_position;
                        character._target_angular_position = _current_rotation;
                        character._target_linear_velocity = glm::vec3(0);
                        character._target_angular_velocity = glm::vec3(0);
                        return;
                    }

                    const glm::vec3 p_now = convert(_bullet_transform.getOrigin());
                    const btQuaternion bq = _bullet_transform.getRotation();
                    const glm::quat q_now(bq.getW(), bq.getX(), bq.getY(), bq.getZ());

                    // Accumulators if you have multiple motion tracks (blend by controller weight)
                    glm::vec3 sum_dpos_xy(0.0f); // desired horizontal displacement this frame
                    glm::vec3 sum_v_xy(0.0f); // desired horizontal velocity
                    float sum_yaw_rate = 0.0f; // rad/s around up
                    float sum_w = 0.0f;

                    for (const std::pair<const std::string, fetched_container<motion_track>>& _pair : animator._motion_tracks) {
                        if (_pair.second.has_value()) {
                            const motion_track& _track = _pair.second.value();
                            const animation_controller& _controller = animator._controllers.at(_pair.first);
                            if (_controller._computed_weight > 0.f) {
                                const glm::mat4 _delta_motion_transform = _compute_motion_delta(_track, _controller._time_ratio, _controller._last_time_ratio, _controller._computed_weight, _controller._has_looped);

                                // Per-frame delta in track space
                                glm::vec3 dpos_local = glm::vec3(_delta_motion_transform[3]); // in *root bone local space*
                                glm::quat drot_local = glm::quat_cast(_delta_motion_transform);
                                glm::vec3 dpos_xy = project_on_plane(q_now * dpos_local, _world_up);
                                glm::vec3 v_des_xy = dpos_xy / _delta_time;
                                glm::quat q_world_after = q_now * drot_local;
                                glm::quat q_delta_world = q_world_after * glm::inverse(q_now);
                                const glm::vec3 fwd = glm::normalize(q_delta_world * _world_forward);
                                const glm::vec3 fwd_xy = glm::normalize(project_on_plane(fwd, _world_up));
                                const glm::vec3 right = glm::normalize(glm::cross(_world_up, fwd_xy));
                                const glm::mat3 R = glm::mat3(right, _world_up, fwd_xy);
                                glm::quat dq = glm::quat_cast(glm::mat4(R));
                                if (dq.w < 0) {
                                    dq = { -dq.w, -dq.x, -dq.y, -dq.z };
                                }
                                float angle = 2.f * std::acos(glm::clamp(dq.w, 0.f, 1.f));
                                glm::vec3 axis = (std::abs(angle) < 1e-6f) ? _world_up : glm::normalize(glm::vec3(dq.x, dq.y, dq.z));
                                float yaw_rate = (glm::dot(axis, _world_up)) * (angle / _delta_time);
                                sum_dpos_xy += dpos_xy;
                                sum_v_xy += v_des_xy;
                                sum_yaw_rate += yaw_rate;
                                sum_w += 1.f;
                            }
                        }
                    }
                    if (sum_w > 0.f) {
                        sum_dpos_xy /= sum_w;
                        sum_v_xy /= sum_w;
                        sum_yaw_rate /= sum_w;
                    }

                    character._target_linear_position = p_now + sum_dpos_xy;
                    character._target_angular_position = glm::normalize(glm::angleAxis(sum_yaw_rate * _delta_time, _world_up) * q_now);
                    character._target_linear_velocity = sum_v_xy;
                    character._target_angular_velocity = _world_up * sum_yaw_rate;
                }
            });
        });
    }

    void motion_system::collect_debug_guizmos()
    {
#if LUCARIA_GUIZMO
        detail::each_scene([](entt::registry& scene) {
            // transform guizmos
            scene.view<transform_component>().each([](transform_component& transform) {
                constexpr glm::float32 _line_length = .2f;

                const glm::vec3 _origin = glm::vec3(transform._transform[3]);

                const glm::vec3 _xaxis = glm::normalize(glm::vec3(transform._transform[0]));
                const glm::vec3 _yaxis = glm::normalize(glm::vec3(transform._transform[1]));
                const glm::vec3 _zaxis = glm::normalize(glm::vec3(transform._transform[2]));

                const glm::vec3 _xendpoint = _origin + _xaxis * _line_length;
                const glm::vec3 _yendpoint = _origin + _yaxis * _line_length;
                const glm::vec3 _zendpoint = _origin + _zaxis * _line_length;

                draw_guizmo_line(convert_bullet(_origin), convert_bullet(_xendpoint), btVector3(1, 0, 0)); // red
                draw_guizmo_line(convert_bullet(_origin), convert_bullet(_yendpoint), btVector3(0, 1, 0)); // green
                draw_guizmo_line(convert_bullet(_origin), convert_bullet(_zendpoint), btVector3(0, 0, 1)); // blue
            });

            // animator guizmos
            scene.view<animator_component>(entt::exclude<transform_component>).each([](animator_component& animator) {
                if (animator._skeleton.has_value()) {
                    const ozz::vector<ozz::math::Float4x4>& _model_transforms = animator._model_transforms;

                    if (!_model_transforms.empty()) {
                        const ozz::span<const std::int16_t>& _joint_parents = animator._skeleton.value().get_handle().joint_parents();

                        if (_model_transforms.size() != _joint_parents.size()) {
                            LUCARIA_RUNTIME_ERROR("Mismatch between model transforms and joint parents sizes")
                        }

                        for (std::size_t _index = 0; _index < _model_transforms.size(); ++_index) {
                            const int _parent_index = _joint_parents[_index];
                            if (_parent_index == ozz::animation::Skeleton::kNoParent) {
                                continue;
                            }

                            const ozz::math::Float4x4& _current_transform = _model_transforms[_index];
                            const ozz::math::Float4x4& _parent_transform = _model_transforms[_parent_index];

                            const btVector3 _from(
                                ozz::math::GetX(_parent_transform.cols[3]),
                                ozz::math::GetY(_parent_transform.cols[3]),
                                ozz::math::GetZ(_parent_transform.cols[3]));
                            const btVector3 _to(
                                ozz::math::GetX(_current_transform.cols[3]),
                                ozz::math::GetY(_current_transform.cols[3]),
                                ozz::math::GetZ(_current_transform.cols[3]));

                            detail::draw_guizmo_line(_from, _to, btVector3(0, 1, 1)); // cyan
                        }
                    }
                }
            });

            scene.view<animator_component, transform_component>().each([](animator_component& animator, transform_component& transform) {
                if (animator._skeleton.has_value()) {
                    const ozz::vector<ozz::math::Float4x4>& _model_transforms = animator._model_transforms;

                    if (!_model_transforms.empty()) {
                        const ozz::span<const std::int16_t>& _joint_parents = animator._skeleton.value().get_handle().joint_parents();

                        if (_model_transforms.size() != _joint_parents.size()) {
                            LUCARIA_RUNTIME_ERROR("Mismatch between model transforms and joint parents sizes")
                        }

                        for (std::size_t _index = 0; _index < _model_transforms.size(); ++_index) {
                            const int _parent_index = _joint_parents[_index];
                            if (_parent_index == ozz::animation::Skeleton::kNoParent) {
                                continue;
                            }

                            const ozz::math::Float4x4 _modifier_transform = convert_ozz(transform._transform);
                            const ozz::math::Float4x4 _current_transform = _modifier_transform * _model_transforms[_index];
                            const ozz::math::Float4x4 _parent_transform = _modifier_transform * _model_transforms[_parent_index];

                            const btVector3 _from(
                                ozz::math::GetX(_parent_transform.cols[3]),
                                ozz::math::GetY(_parent_transform.cols[3]),
                                ozz::math::GetZ(_parent_transform.cols[3]));
                            const btVector3 _to(
                                ozz::math::GetX(_current_transform.cols[3]),
                                ozz::math::GetY(_current_transform.cols[3]),
                                ozz::math::GetZ(_current_transform.cols[3]));

                            detail::draw_guizmo_line(_from, _to, btVector3(0, 1, 1)); // cyan
                        }
                    }
                }
            });
        });
#endif
    }
}
}
