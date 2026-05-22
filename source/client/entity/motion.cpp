#include <glm/gtx/matrix_interpolation.hpp>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/error.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/window.hpp>
#include <lucaria/entity/animator.hpp>
#include <lucaria/entity/model.hpp>
#include <lucaria/entity/motion.hpp>
#include <lucaria/entity/rigidbody.hpp>
#include <lucaria/entity/scene.hpp>
#include <lucaria/entity/transform.hpp>

namespace lucaria {

#if defined(LUCARIA_DEBUG)
extern void _draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);
#endif

namespace detail {

    namespace {

        static const float32x3 _world_up = float32x3(0, 1, 0);
        static const float32x3 _world_forward = float32x3(0, 0, -1);

        [[nodiscard]] float32x4x4 _sample_motion_track(const motion_track_implementation& track, const float32 ratio)
        {
            // position sampling
            ozz::math::Transform _ozz_affine_transform;
            ozz::animation::Float3TrackSamplingJob _position_sampler;
            _position_sampler.track = &track.translation_track;
            _position_sampler.result = &_ozz_affine_transform.translation;
            _position_sampler.ratio = ratio;
            if (!_position_sampler.Run()) {
                LUCARIA_RUNTIME_ERROR("Failed to run vec3 track sampling job")
            }

            // rotation sampling
            ozz::animation::QuaternionTrackSamplingJob _rotation_sampler;
            _rotation_sampler.track = &track.rotation_track;
            _rotation_sampler.result = &_ozz_affine_transform.rotation;
            _rotation_sampler.ratio = ratio;
            if (!_rotation_sampler.Run()) {
                LUCARIA_RUNTIME_ERROR("Failed to run quat track sampling job")
            }

            _ozz_affine_transform.scale = ozz::math::Float3::one();
            return convert(ozz::math::Float4x4::FromAffine(_ozz_affine_transform));
        }

        [[nodiscard]] float32x4x4 _compute_motion_delta(const motion_track_implementation& track, const float32 time_ratio, const float32 last_time_ratio, const float32 weight, const bool has_looped)
        {
            // base delta
            const glm::mat4 _new_motion_transform = _sample_motion_track(track, time_ratio);
            const glm::mat4 _last_motion_transform = _sample_motion_track(track, last_time_ratio);
            glm::mat4 _delta_motion_transform = _new_motion_transform * glm::inverse(_last_motion_transform);

            // loop delta
            if (has_looped) {
                const glm::mat4 _end_motion_transform = _sample_motion_track(track, 1.f);
                const glm::mat4 _begin_motion_transform = _sample_motion_track(track, 0.f);
                _delta_motion_transform = _end_motion_transform * glm::inverse(_begin_motion_transform) * _delta_motion_transform;
            }

            // weight interpolation
            _delta_motion_transform = glm::interpolate(glm::mat4(1.f), _delta_motion_transform, weight);
            return _delta_motion_transform;
        }

    }

    void motion_system::advance_controllers()
    {
        each_view<animator_component>([](animator_component& animator) {
            for (std::pair<const std::string, animation_controller>& _pair : animator._controllers) {

                // advance time
                animation_controller& _controller = _pair.second;
                _controller._last_time_ratio = _controller._time_ratio;
                if (_controller._is_playing) {
                    _controller._time_ratio += _controller._playback_speed * static_cast<glm::float32>(engine_window().time_delta_seconds);
                }
                _controller._has_looped = _controller._time_ratio > 1.f;
                _controller._time_ratio = glm::mod(_controller._time_ratio, 1.f);

                // fire events
                typename std::unordered_map<std::string, lucaria::event_track_object>::const_iterator _event_track_it = animator._event_tracks.find(_pair.first);
                if (_controller._is_playing && _event_track_it != animator._event_tracks.end() && _event_track_it->second) {
                    for (const event_data& _event : _event_track_it->second._resource->fetched.value().data.events) {
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
    }

    void motion_system::apply_animations()
    {
        each_view<animator_component>([](animator_component& animator) {
            if (animator._skeleton) {

                // sampling
                ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers;
                for (std::pair<const std::string, animation_object>& _pair : animator._animations) {
                    if (_pair.second) {
                        const animation_controller& _controller = animator._controllers.at(_pair.first);
                        ozz::vector<ozz::math::SoaTransform>& _local_transforms = animator._local_transforms.at(_pair.first);
                        ozz::animation::SamplingJob sampling_job;
                        sampling_job.animation = &_pair.second._resource->fetched.value().animation;
                        sampling_job.context = animator._sampling_context.get();
                        sampling_job.ratio = _controller._time_ratio;
                        sampling_job.output = make_span(_local_transforms);
                        if (!sampling_job.Run()) {
                            LUCARIA_RUNTIME_ERROR("Failed to run animation sampling job")
                        }

                        // prepare blend layers
                        ozz::animation::BlendingJob::Layer& _blend_layer = _blend_layers.emplace_back();
                        _blend_layer.transform = make_span(_local_transforms);
                        _blend_layer.weight = _controller._weight;
                    }
                }

                // blending
                ozz::animation::Skeleton& _skeleton = animator._skeleton._resource->fetched.value().skeleton;
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
    }

    void motion_system::apply_motion_tracks()
    {
        // apply to transform if no dynamic rigidbody
        each_view<const animator_component, transform_component>(entt::exclude<dynamic_rigidbody_component>, [](const animator_component& _animator, transform_component& _transform) {
            for (const std::pair<const std::string, motion_track_object>& _pair : _animator._motion_tracks) {
                if (_pair.second) {
                    const animation_controller& _controller = _animator._controllers.at(_pair.first);
                    if (_controller._weight > 0.f) {
                        const detail::motion_track_implementation& _track = _pair.second._resource->fetched.value();
                        const glm::mat4 _delta_transform = detail::_compute_motion_delta(_track, _controller._time_ratio, _controller._last_time_ratio, _controller._weight, _controller._has_looped);
                        _transform.set_transform_relative(_delta_transform);
                    }
                }
            }
        });

        // compute and apply to PD targets if dynamic rigidbody
        const glm::float32 _delta_time = static_cast<glm::float32>(engine_window().time_delta_seconds);
        each_view<const animator_component, transform_component, dynamic_rigidbody_component>([_delta_time](const animator_component& animator, transform_component& transform, dynamic_rigidbody_component& rigidbody) {
            if (rigidbody._shape) {
                btRigidBody* _bullet_rigidbody = rigidbody._rigidbody.get();
                const glm::mat4 _transform_matrix = rigidbody._shape._resource->fetched.value().feet_to_center * transform._transform;
                const glm::vec3 _current_position = glm::vec3(_transform_matrix[3]);
                const glm::quat _current_rotation = glm::quat_cast(_transform_matrix);
                const btTransform _bullet_transform = convert_bullet(_transform_matrix);
                // _bullet_rigidbody->setInterpolationWorldTransform(_bullet_transform);
                _bullet_rigidbody->setWorldTransform(_bullet_transform);

                // reset on teleporting
                if (rigidbody._last_position != transform.get_position()) {
                    // _bullet_rigidbody->setInterpolationLinearVelocity(btVector3(0, 0, 0));
                    _bullet_rigidbody->setLinearVelocity(btVector3(0, 0, 0));
                    // _bullet_rigidbody->setInterpolationAngularVelocity(btVector3(0, 0, 0));
                    _bullet_rigidbody->setAngularVelocity(btVector3(0, 0, 0));
                    _bullet_rigidbody->clearForces();
                    _bullet_rigidbody->activate(true);
                    rigidbody._target_linear_position = _current_position;
                    rigidbody._target_angular_position = _current_rotation;
                    rigidbody._target_linear_velocity = glm::vec3(0);
                    rigidbody._target_angular_velocity = glm::vec3(0);
                    return;
                }

                // compute and sum for each motion track
                glm::vec3 _sum_displacement_xy = glm::vec3(0);
                glm::vec3 _sum_velocity_xy = glm::vec3(0);
                glm::float32 _sum_velocity_yaw = 0.f;
                glm::float32 _sum_blend_size = 0.f;
                for (const std::pair<const std::string, motion_track_object>& _pair : animator._motion_tracks) {
                    if (_pair.second) {
                        const detail::motion_track_implementation& _track = _pair.second._resource->fetched.value();
                        const animation_controller& _controller = animator._controllers.at(_pair.first);
                        if (_controller._weight > 0.f) {
                            const glm::mat4 _delta_motion_transform = detail::_compute_motion_delta(_track, _controller._time_ratio, _controller._last_time_ratio, _controller._weight, _controller._has_looped);
                            const glm::vec3 _delta_position_xy = project_on_plane(_current_rotation * glm::vec3(_delta_motion_transform[3]), _world_up);
                            const glm::vec3 _linear_velocity_xy = _delta_position_xy / _delta_time;
                            const glm::vec3 _forward_xy = glm::normalize(project_on_plane(glm::normalize(glm::quat_cast(_delta_motion_transform) * _world_forward), _world_up));
                            glm::quat _delta_rotation_yaw = glm::quat_cast(glm::mat3(glm::normalize(glm::cross(_world_up, _forward_xy)), _world_up, _forward_xy));
                            if (_delta_rotation_yaw.w < 0) {
                                _delta_rotation_yaw = -_delta_rotation_yaw;
                            }
                            const glm::float32 _angle_yaw = 2.f * std::acos(glm::clamp(_delta_rotation_yaw.w, 0.f, 1.f));
                            const glm::vec3 _axis_yaw = (std::abs(_angle_yaw) < 1e-6f) ? _world_up : glm::normalize(glm::vec3(_delta_rotation_yaw.x, _delta_rotation_yaw.y, _delta_rotation_yaw.z));
                            const glm::float32 _angular_velocity_yaw = (glm::dot(_axis_yaw, _world_up)) * (_angle_yaw / _delta_time);
                            _sum_displacement_xy += _delta_position_xy;
                            _sum_velocity_xy += _linear_velocity_xy;
                            _sum_velocity_yaw += _angular_velocity_yaw;
                            _sum_blend_size += 1.f;
                        }
                    }
                }

                // blend motion tracks
                if (_sum_blend_size > 0.f) {
                    _sum_displacement_xy /= _sum_blend_size;
                    _sum_velocity_xy /= _sum_blend_size;
                    _sum_velocity_yaw /= _sum_blend_size;
                }
                rigidbody._target_linear_position = _current_position + _sum_displacement_xy;
                rigidbody._target_linear_velocity = _sum_velocity_xy;
                rigidbody._target_angular_position = glm::normalize(glm::angleAxis(_sum_velocity_yaw * _delta_time, _world_up) * _current_rotation);
                rigidbody._target_angular_velocity = _world_up * _sum_velocity_yaw;
            }
        });
    }

    void motion_system::collect_debug_guizmos()
    {
#if defined(LUCARIA_DEBUG)
        each_view<transform_component>([](transform_component& transform) {
            constexpr glm::float32 _line_length = 1.2f;
            const glm::vec3 _origin = transform.get_position();
            const glm::vec3 _xaxis = glm::normalize(glm::vec3(transform._transform[0]));
            const glm::vec3 _yaxis = glm::normalize(glm::vec3(transform._transform[1]));
            const glm::vec3 _zaxis = glm::normalize(glm::vec3(transform._transform[2]));
            const glm::vec3 _xendpoint = _origin + _xaxis * _line_length;
            const glm::vec3 _yendpoint = _origin + _yaxis * _line_length;
            const glm::vec3 _zendpoint = _origin + _zaxis * _line_length;
            _draw_guizmo_line(convert_bullet(_origin), convert_bullet(_xendpoint), btVector3(1, 0, 0)); // red
            _draw_guizmo_line(convert_bullet(_origin), convert_bullet(_yendpoint), btVector3(0, 1, 0)); // green
            _draw_guizmo_line(convert_bullet(_origin), convert_bullet(_zendpoint), btVector3(0, 0, 1)); // blue
        });

        // animator guizmos without transforms
        each_view<animator_component>(entt::exclude<transform_component>, [](animator_component& animator) {
            if (animator._skeleton) {
                const ozz::vector<ozz::math::Float4x4>& _model_transforms = animator._model_transforms;
                if (_model_transforms.empty()) {
                    return;
                }
                const ozz::span<const std::int16_t>& _joint_parents = animator._skeleton._resource->fetched.value().skeleton.joint_parents();
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
                    const btVector3 _from(ozz::math::GetX(_parent_transform.cols[3]), ozz::math::GetY(_parent_transform.cols[3]), ozz::math::GetZ(_parent_transform.cols[3]));
                    const btVector3 _to(ozz::math::GetX(_current_transform.cols[3]), ozz::math::GetY(_current_transform.cols[3]), ozz::math::GetZ(_current_transform.cols[3]));
                    _draw_guizmo_line(_from, _to, btVector3(0, 1, 1)); // cyan
                }
            }
        });

        // animator guizmos with transforms
        each_view<animator_component, transform_component>([](animator_component& animator, transform_component& transform) {
            if (animator._skeleton) {
                const ozz::vector<ozz::math::Float4x4>& _model_transforms = animator._model_transforms;
                if (_model_transforms.empty()) {
                    return;
                }
                const ozz::span<const std::int16_t>& _joint_parents = animator._skeleton._resource->fetched.value().skeleton.joint_parents();
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
                    const btVector3 _from(ozz::math::GetX(_parent_transform.cols[3]), ozz::math::GetY(_parent_transform.cols[3]), ozz::math::GetZ(_parent_transform.cols[3]));
                    const btVector3 _to(ozz::math::GetX(_current_transform.cols[3]), ozz::math::GetY(_current_transform.cols[3]), ozz::math::GetZ(_current_transform.cols[3]));
                    _draw_guizmo_line(_from, _to, btVector3(0, 1, 1)); // cyan
                }
            }
        });
#endif
    }

}
}
