#include <glm/gtx/matrix_interpolation.hpp>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/track_sampling_job.h>

#include <lucaria/core/systems_rendering.hpp>
#include <lucaria/core/systems_motion.hpp>
#include <lucaria/core/utils_math.hpp>
#include <lucaria/engine/component_animator.hpp>
#include <lucaria/engine/component_model.hpp>
#include <lucaria/engine/component_rigidbody.hpp>
#include <lucaria/engine/component_transform.hpp>

namespace lucaria {
namespace detail {

    namespace {

        [[nodiscard]] float32x4x4 _sample_motion_track(const object_motion_track& track, const float32 ratio)
        {
            // position sampling
            ozz::math::Transform _ozz_affine_transform;
            ozz::animation::Float3TrackSamplingJob _position_sampler;
            _position_sampler.track = &track.translation_track;
            _position_sampler.result = &_ozz_affine_transform.translation;
            _position_sampler.ratio = ratio;
            if (!_position_sampler.Run()) {
                LUCARIA_DEBUG_ERROR("Failed to run vec3 track sampling job")
            }

            // rotation sampling
            ozz::animation::QuaternionTrackSamplingJob _rotation_sampler;
            _rotation_sampler.track = &track.rotation_track;
            _rotation_sampler.result = &_ozz_affine_transform.rotation;
            _rotation_sampler.ratio = ratio;
            if (!_rotation_sampler.Run()) {
                LUCARIA_DEBUG_ERROR("Failed to run quat track sampling job")
            }

            _ozz_affine_transform.scale = ozz::math::Float3::one();
            return convert(ozz::math::Float4x4::FromAffine(_ozz_affine_transform));
        }

        [[nodiscard]] float32x4x4 _compute_motion_delta(const object_motion_track& track, const float32 time_ratio, const float32 last_time_ratio, const float32 weight, const bool has_looped)
        {
            // base delta
            const float32x4x4 _new_motion_transform = _sample_motion_track(track, time_ratio);
            const float32x4x4 _last_motion_transform = _sample_motion_track(track, last_time_ratio);
            float32x4x4 _delta_motion_transform = _new_motion_transform * glm::inverse(_last_motion_transform);

            // loop delta
            if (has_looped) {
                const float32x4x4 _end_motion_transform = _sample_motion_track(track, 1.f);
                const float32x4x4 _begin_motion_transform = _sample_motion_track(track, 0.f);
                _delta_motion_transform = _end_motion_transform * glm::inverse(_begin_motion_transform) * _delta_motion_transform;
            }

            // weight interpolation
            _delta_motion_transform = glm::interpolate(float32x4x4(1.f), _delta_motion_transform, weight);
            return _delta_motion_transform;
        }

    }

    void system_motion::update_advance_controllers(manager_window& window, manager_scenes& scenes)
    {
        const float32 _delta_time = static_cast<float32>(window.time_delta_seconds);
        scenes.each_view<component_animator>([_delta_time](component_animator& animator) {
            for (std::pair<const std::string, component_animator_controller>& _pair : animator._controllers) {

                // advance time
                component_animator_controller& _controller = _pair.second;
                _controller._last_time_ratio = _controller._time_ratio;
                if (_controller._is_playing) {
                    _controller._time_ratio += _controller._playback_speed * _delta_time;
                }
                _controller._has_looped = _controller._time_ratio > 1.f;
                _controller._time_ratio = glm::mod(_controller._time_ratio, 1.f);

                // fire events
                typename std::unordered_map<std::string, lucaria::handle_event_track>::const_iterator _event_track_it = animator._event_tracks.find(_pair.first);
                if (_controller._is_playing && _event_track_it != animator._event_tracks.end() && _event_track_it->second) {
                    for (const data_event& _event : _event_track_it->second._cached->fetched.value().data.events) {
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

    void system_motion::update_apply_animations(manager_scenes& scenes)
    {
        scenes.each_view<component_animator>([](component_animator& animator) {
            if (animator._skeleton) {

                // sampling
                ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers;
                for (std::pair<const std::string, handle_animation>& _pair : animator._animations) {
                    if (_pair.second) {
                        const component_animator_controller& _controller = animator._controllers.at(_pair.first);
                        ozz::vector<ozz::math::SoaTransform>& _local_transforms = animator._local_transforms.at(_pair.first);
                        ozz::animation::SamplingJob sampling_job;
                        sampling_job.animation = &_pair.second._cached->fetched.value().animation;
                        sampling_job.context = animator._sampling_context.get();
                        sampling_job.ratio = _controller._time_ratio;
                        sampling_job.output = make_span(_local_transforms);
                        if (!sampling_job.Run()) {
                            LUCARIA_DEBUG_ERROR("Failed to run animation sampling job")
                        }

                        // prepare blend layers
                        ozz::animation::BlendingJob::Layer& _blend_layer = _blend_layers.emplace_back();
                        _blend_layer.transform = make_span(_local_transforms);
                        _blend_layer.weight = _controller._weight;
                    }
                }

                // blending
                ozz::animation::Skeleton& _skeleton = animator._skeleton._cached->fetched.value().skeleton;
                ozz::animation::BlendingJob _blending_job;
                _blending_job.threshold = 0.1f; // TODO let user set parameter
                _blending_job.additive_layers = {};
                _blending_job.layers = make_span(_blend_layers);
                _blending_job.rest_pose = _skeleton.joint_rest_poses();
                _blending_job.output = make_span(animator._blended_local_transforms);
                if (!_blending_job.Run()) {
                    LUCARIA_DEBUG_ERROR("Failed to run blending job")
                }

                // local to model
                ozz::animation::LocalToModelJob _local_to_model_job;
                _local_to_model_job.skeleton = &_skeleton;
                _local_to_model_job.input = make_span(animator._blended_local_transforms);
                _local_to_model_job.output = make_span(animator._model_transforms);
                if (!_local_to_model_job.Run()) {
                    LUCARIA_DEBUG_ERROR("Failed to run local to model job")
                }
            }
        });
    }

    void system_motion::update_apply_motion_tracks(manager_window& window, manager_scenes& scenes)
    {
        // apply to transform if no dynamic rigidbody
        scenes.each_view<const component_animator, component_transform>(exclude<component_rigidbody_dynamic>, [](const component_animator& _animator, component_transform& _transform) {
            for (const std::pair<const std::string, handle_motion_track>& _pair : _animator._motion_tracks) {
                if (_pair.second) {
                    const component_animator_controller& _controller = _animator._controllers.at(_pair.first);
                    if (_controller._weight > 0.f) {
                        const object_motion_track& _track = _pair.second._cached->fetched.value();
                        const float32x4x4 _delta_transform = _compute_motion_delta(_track, _controller._time_ratio, _controller._last_time_ratio, _controller._weight, _controller._has_looped);
                        _transform.set_transform_relative(_delta_transform);
                    }
                }
            }
        });

        // compute and apply to PD targets if dynamic rigidbody
        const float32 _delta_time = static_cast<float32>(window.time_delta_seconds);
        scenes.each_view<const component_animator, component_transform, component_rigidbody_dynamic>([this, _delta_time](const component_animator& animator, component_transform& transform, component_rigidbody_dynamic& rigidbody) {
            if (rigidbody._shape) {
                btRigidBody* _bullet_rigidbody = rigidbody._rigidbody.get();
                const float32x4x4 _transform_matrix = rigidbody._shape._cached->fetched.value().feet_to_center * transform._transform;
                const float32x3 _current_position = float32x3(_transform_matrix[3]);
                const float32quat _current_rotation = glm::quat_cast(_transform_matrix);
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
                    rigidbody._target_linear_velocity = float32x3(0);
                    rigidbody._target_angular_velocity = float32x3(0);
                    return;
                }

                // compute and sum for each motion track
                float32x3 _sum_displacement_xy = float32x3(0);
                float32x3 _sum_velocity_xy = float32x3(0);
                float32 _sum_velocity_yaw = 0.f;
                float32 _sum_blend_size = 0.f;
                for (const std::pair<const std::string, handle_motion_track>& _pair : animator._motion_tracks) {
                    if (_pair.second) {
                        const object_motion_track& _track = _pair.second._cached->fetched.value();
                        const component_animator_controller& _controller = animator._controllers.at(_pair.first);
                        if (_controller._weight > 0.f) {
                            const float32x4x4 _delta_motion_transform = _compute_motion_delta(_track, _controller._time_ratio, _controller._last_time_ratio, _controller._weight, _controller._has_looped);
                            const float32x3 _delta_position_xy = project_on_plane(_current_rotation * float32x3(_delta_motion_transform[3]), world_up);
                            const float32x3 _linear_velocity_xy = _delta_position_xy / _delta_time;
                            const float32x3 _forward_xy = glm::normalize(project_on_plane(glm::normalize(glm::quat_cast(_delta_motion_transform) * world_forward), world_up));
                            float32quat _delta_rotation_yaw = glm::quat_cast(float32x3x3(glm::normalize(glm::cross(world_up, _forward_xy)), world_up, _forward_xy));
                            if (_delta_rotation_yaw.w < 0) {
                                _delta_rotation_yaw = -_delta_rotation_yaw;
                            }
                            const float32 _angle_yaw = 2.f * std::acos(glm::clamp(_delta_rotation_yaw.w, 0.f, 1.f));
                            const float32x3 _axis_yaw = (std::abs(_angle_yaw) < 1e-6f) ? world_up : glm::normalize(float32x3(_delta_rotation_yaw.x, _delta_rotation_yaw.y, _delta_rotation_yaw.z));
                            const float32 _angular_velocity_yaw = (glm::dot(_axis_yaw, world_up)) * (_angle_yaw / _delta_time);
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
                rigidbody._target_angular_position = glm::normalize(glm::angleAxis(_sum_velocity_yaw * _delta_time, world_up) * _current_rotation);
                rigidbody._target_angular_velocity = world_up * _sum_velocity_yaw;
            }
        });
    }

    void system_motion::update_collect_debug_guizmos(system_rendering& rendering, manager_scenes& scenes)
    {
#if defined(LUCARIA_DEBUG)
		if (!rendering.show_physics_guizmos) {
			return;
		}
        scenes.each_view<component_transform>([&rendering](component_transform& transform) {
            constexpr float32 _line_length = 1.2f;
            const float32x3 _origin = transform.get_position();
            const float32x3 _xaxis = glm::normalize(float32x3(transform._transform[0]));
            const float32x3 _yaxis = glm::normalize(float32x3(transform._transform[1]));
            const float32x3 _zaxis = glm::normalize(float32x3(transform._transform[2]));
            const float32x3 _xendpoint = _origin + _xaxis * _line_length;
            const float32x3 _yendpoint = _origin + _yaxis * _line_length;
            const float32x3 _zendpoint = _origin + _zaxis * _line_length;
            rendering.guizmo_draw.drawLine(convert_bullet(_origin), convert_bullet(_xendpoint), btVector3(1, 0, 0)); // red
            rendering.guizmo_draw.drawLine(convert_bullet(_origin), convert_bullet(_yendpoint), btVector3(0, 1, 0)); // green
            rendering.guizmo_draw.drawLine(convert_bullet(_origin), convert_bullet(_zendpoint), btVector3(0, 0, 1)); // blue
        });

        // animator guizmos without transforms
        scenes.each_view<component_animator>(exclude<component_transform>, [&rendering](component_animator& animator) {
            if (animator._skeleton) {
                const ozz::vector<ozz::math::Float4x4>& _model_transforms = animator._model_transforms;
                if (_model_transforms.empty()) {
                    return;
                }
                const ozz::span<const std::int16_t>& _joint_parents = animator._skeleton._cached->fetched.value().skeleton.joint_parents();
                if (_model_transforms.size() != _joint_parents.size()) {
                    LUCARIA_DEBUG_ERROR("Mismatch between model transforms and joint parents sizes")
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
                    rendering.guizmo_draw.drawLine(_from, _to, btVector3(0, 1, 1)); // cyan
                }
            }
        });

        // animator guizmos with transforms
        scenes.each_view<component_animator, component_transform>([&rendering](component_animator& animator, component_transform& transform) {
            if (animator._skeleton) {
                const ozz::vector<ozz::math::Float4x4>& _model_transforms = animator._model_transforms;
                if (_model_transforms.empty()) {
                    return;
                }
                const ozz::span<const std::int16_t>& _joint_parents = animator._skeleton._cached->fetched.value().skeleton.joint_parents();
                if (_model_transforms.size() != _joint_parents.size()) {
                    LUCARIA_DEBUG_ERROR("Mismatch between model transforms and joint parents sizes")
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
                    rendering.guizmo_draw.drawLine(_from, _to, btVector3(0, 1, 1)); // cyan
                }
            }
        });
#endif
    }

}
}
