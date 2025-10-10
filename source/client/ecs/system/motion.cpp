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
#include <lucaria/ecs/component/rigidbody.hpp>
#include <lucaria/ecs/component/transform.hpp>
#include <lucaria/ecs/system/motion.hpp>

namespace lucaria {
namespace {

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

#if LUCARIA_GUIZMO
    extern void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);
#endif

    void motion_system::advance_controllers()
    {
        detail::each_scene([](entt::registry& scene) {
            scene.view<ecs::animator_component>().each([](ecs::animator_component& animator) {
                for (std::pair<const std::string, ecs::animation_controller>& _pair : animator._controllers) {

                    ecs::animation_controller& _controller = _pair.second;
                    if (_controller._is_playing) {
                        _controller._last_time_ratio = _controller._time_ratio;
                        _controller._time_ratio += _controller._playback_speed * static_cast<float>(get_time_delta()); // * 0.1f; // c'est bien le temps qu'il faut multiplier par le weight des fadeins
                    }
                    _controller._has_looped = _controller._time_ratio > 1.f;
                    _controller._time_ratio = glm::mod(_controller._time_ratio, 1.f);
                    _controller._computed_weight = _controller._weight; // add fade in and fade out

                    // if we passed an event trigger
                    if (_controller._is_playing && animator._event_tracks[_pair.first].has_value()) {
                        const float last = _controller._last_time_ratio; // in [0,1)
                        const float curr = _controller._time_ratio; // in [0,1)

                        // crossing test: open-left / closed-right interval (last, curr], with wrap
                        auto crossed = [&](float t) -> bool {
                            // assume t is already in [0,1]
                            if (last <= curr) {
                                return (last < t) && (t <= curr);
                            } else {
                                // wrapped: (last,1] U (0,curr]
                                return (last < t) || (t <= curr);
                            }
                        };

                        // get events from the controller's fetched event track
                        const lucaria::event_track& track = animator._event_tracks[_pair.first].value(); // event_track&
                        const std::vector<lucaria::event_data>& events = track.data.events; // std::vector<event_data>

                        for (const auto& ev : events) {
                            const float t = glm::clamp(ev.time_normalized, 0.f, 1.f);
                            if (!crossed(t))
                                continue;

                            // call registered callback if any
                            if (auto it = _controller._event_callbacks.find(ev.name);
                                it != _controller._event_callbacks.end() && it->second) {
                                it->second();
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
        detail::each_scene([](entt::registry& scene) {
            scene.view<const ecs::animator_component, ecs::transform_component>(entt::exclude<ecs::character_rigidbody_component>).each([](const ecs::animator_component& animator, ecs::transform_component& transform) {
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

                        _delta_transform = glm::interpolate(glm::mat4(1.f), _delta_transform, _controller._computed_weight);
                        transform.set_transform_relative(_delta_transform);
                    }
                }
            });
        });

        detail::each_scene([](entt::registry& scene) {
            // characters
            scene.view<const ecs::animator_component, ecs::transform_component, ecs::character_rigidbody_component>().each([](const ecs::animator_component& animator, ecs::transform_component& transform, ecs::character_rigidbody_component& character) {
                btRigidBody* body = character._rigidbody.get();
                if (!body)
                    return;

                const float dt = float(get_time_delta());
                if (dt <= 0.f)
                    return;

                if (character._pending_teleport) {

                    if (!character._shape.has_value())
                        return;
                    character._pending_teleport = false;

                    const glm::mat4 M = character._shape.value().get_feet_to_center() * transform._transform;
                    const btTransform Tb = convert_bullet(M);
                    body->setWorldTransform(Tb);
                    body->getMotionState()->setWorldTransform(Tb);
                    body->setInterpolationWorldTransform(Tb);

                    body->setLinearVelocity(btVector3(0, 0, 0));
                    body->setAngularVelocity(btVector3(0, 0, 0));
                    body->clearForces();
                    body->activate(true);

                    // sync PD targets to avoid elastic correction
                    character._p_d = glm::vec3(M[3]); // translation part
                    character._q_d = glm::quat_cast(M); // orientation
                    character._v_d = glm::vec3(0);
                    character._w_d = glm::vec3(0);
                    return;
                }

                const btTransform Tb = convert_bullet(character._shape.value().get_feet_to_center() * transform._transform);
                body->setWorldTransform(Tb);

                //                 btTransform Tb = body->getWorldTransform();

                // // keep translation from Bullet
                // btVector3 pos = Tb.getOrigin();

                // // compute new orientation (only yaw from camera)
                // btQuaternion qNow = Tb.getRotation();
                // glm::quat q_now(qNow.getW(), qNow.getX(), qNow.getY(), qNow.getZ());

                // // your camera yaw input
                // glm::quat qYawDelta = glm::angleAxis(glm::radians(yaw_delta), glm::vec3(0,1,0));

                // // apply only yaw override
                // glm::quat q_new = qYawDelta * q_now;
                // btQuaternion qNew(q_new.x, q_new.y, q_new.z, q_new.w);

                // // write back
                // Tb.setOrigin(pos);
                // Tb.setRotation(qNew);
                // body->setWorldTransform(Tb);
                // body->getMotionState()->setWorldTransform(Tb);
                // body->setInterpolationWorldTransform(Tb);

                // const btTransform Tb = body->getWorldTransform();
                const glm::vec3 p_now = { Tb.getOrigin().x(), Tb.getOrigin().y(), Tb.getOrigin().z() };
                const btQuaternion bq = Tb.getRotation();
                const glm::quat q_now(bq.getW(), bq.getX(), bq.getY(), bq.getZ());

                const glm::vec3 up = character._up;

                // Accumulators if you have multiple motion tracks (blend by controller weight)
                glm::vec3 sum_dpos_xy(0.0f); // desired horizontal displacement this frame
                glm::vec3 sum_v_xy(0.0f); // desired horizontal velocity
                float sum_yaw_rate = 0.0f; // rad/s around up
                float sum_w = 0.0f;

                for (const auto& kv : animator._motion_tracks) {
                    if (!kv.second.has_value())
                        continue;

                    const motion_track& track = kv.second.value();
                    const auto itCtrl = animator._controllers.find(kv.first);
                    if (itCtrl == animator._controllers.end())
                        continue;
                    const ecs::animation_controller& ctrl = itCtrl->second;
                    const float w = ctrl._computed_weight; // your layer weight
                    if (w <= 0.f)
                        continue;

                    // Sample local root-motion at t0/t1
                    glm::mat4 T0 = sample_motion_track(track, ctrl._last_time_ratio);
                    glm::mat4 T1 = sample_motion_track(track, ctrl._time_ratio);

                    // Handle wrapping of the clip (preserve the authored end->start delta)
                    if (ctrl._has_looped) {
                        glm::mat4 Tend = sample_motion_track(track, 1.f);
                        glm::mat4 Tbeg = sample_motion_track(track, 0.f);
                        T1 = Tend * glm::inverse(Tbeg) * T1;
                    }

                    // Per-frame delta in track space
                    glm::mat4 D = T1 * glm::inverse(T0);
                    glm::vec3 dpos_local = glm::vec3(D[3]); // in *root bone local space*
                    glm::quat drot_local = glm::quat_cast(D);

                    // Rotate displacement into world using current physics orientation
                    glm::vec3 dpos_world = q_now * dpos_local;

                    // Project to ground
                    auto projOnPlane = [&](const glm::vec3& v) { return v - up * glm::dot(up, v); };
                    glm::vec3 dpos_xy = projOnPlane(dpos_world);
                    glm::vec3 v_des_xy = dpos_xy / dt;

                    // Rotate into world by composing with current orientation
                    glm::quat q_world_after = q_now * drot_local;
                    glm::quat q_delta_world = q_world_after * glm::inverse(q_now);

                    auto yawOnly = [&](const glm::quat& q) {
                        const glm::vec3 fwd = glm::normalize(q * glm::vec3(0, 0, 1));
                        const glm::vec3 fwd_xy = glm::normalize(projOnPlane(fwd));
                        const glm::vec3 right = glm::normalize(glm::cross(up, fwd_xy));
                        const glm::mat3 R { right, up, fwd_xy }; // columns
                        return glm::quat_cast(glm::mat4(R));
                    };

                    glm::quat drot_yaw = yawOnly(q_delta_world);

                    // Turn yaw delta into angular speed around up
                    glm::quat dq = drot_yaw;
                    if (dq.w < 0)
                        dq = { -dq.w, -dq.x, -dq.y, -dq.z };

                    float angle = 2.f * std::acos(glm::clamp(dq.w, 0.f, 1.f));
                    glm::vec3 axis = (std::abs(angle) < 1e-6f) ? up : glm::normalize(glm::vec3(dq.x, dq.y, dq.z));
                    float yaw_rate = (glm::dot(axis, up)) * (angle / dt);

                    // Accumulate weighted contributions
                    sum_dpos_xy += w * dpos_xy;
                    sum_v_xy += w * v_des_xy;
                    sum_yaw_rate += w * yaw_rate;
                    sum_w += w;
                }

                // Normalize by total weight if you blended multiple tracks
                if (sum_w > 0.f) {
                    sum_dpos_xy /= sum_w;
                    sum_v_xy /= sum_w;
                    sum_yaw_rate /= sum_w;
                }

                // --- Build absolute PD targets relative to current physics pose ---
                character._p_d = p_now + sum_dpos_xy; // where to be at end of this frame
                // rotate around 'up' by (rate * dt) from current orient:
                auto angleAxis = [&](float ang, const glm::vec3& ax) {
                    return glm::angleAxis(ang, glm::normalize(ax));
                };
                glm::quat q_target = glm::normalize(angleAxis(sum_yaw_rate * dt, up) * q_now);
                character._q_d = q_target;

                character._v_d = sum_v_xy; // desired horizontal velocity
                character._w_d = up * sum_yaw_rate; // desired angular velocity about up
            });
        });
    }

    void motion_system::collect_debug_guizmos()
    {
#if LUCARIA_GUIZMO
        detail::each_scene([](entt::registry& scene) {
            // transform guizmos
            scene.view<ecs::transform_component>().each([](ecs::transform_component& transform) {
                constexpr glm::float32 _line_length = .2f;

                const glm::vec3 _origin = glm::vec3(transform._transform[3]);

                const glm::vec3 _xaxis = glm::normalize(glm::vec3(transform._transform[0]));
                const glm::vec3 _yaxis = glm::normalize(glm::vec3(transform._transform[1]));
                const glm::vec3 _zaxis = glm::normalize(glm::vec3(transform._transform[2]));

                const glm::vec3 _xendpoint = _origin + _xaxis * _line_length;
                const glm::vec3 _yendpoint = _origin + _yaxis * _line_length;
                const glm::vec3 _zendpoint = _origin + _zaxis * _line_length;

                draw_guizmo_line(reinterpret_bullet(_origin), reinterpret_bullet(_xendpoint), btVector3(1, 0, 0)); // red
                draw_guizmo_line(reinterpret_bullet(_origin), reinterpret_bullet(_yendpoint), btVector3(0, 1, 0)); // green
                draw_guizmo_line(reinterpret_bullet(_origin), reinterpret_bullet(_zendpoint), btVector3(0, 0, 1)); // blue
            });

            // animator guizmos
            scene.view<ecs::animator_component>(entt::exclude<ecs::transform_component>).each([](ecs::animator_component& animator) {
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

            scene.view<ecs::animator_component, ecs::transform_component>().each([](ecs::animator_component& animator, ecs::transform_component& transform) {
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

                            const ozz::math::Float4x4& _modifier_transform = reinterpret_ozz(transform._transform);
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
