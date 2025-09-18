#pragma once

#include <future>
#include <memory>
#include <optional>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/world.hpp>
#include <lucaria/ecs/component/transform.hpp>

struct animation_controller {
    animation_controller() = default;
    animation_controller(const animation_controller& other) = delete;
    animation_controller& operator=(const animation_controller& other) = delete;
    animation_controller(animation_controller&& other) = default;
    animation_controller& operator=(animation_controller&& other) = default;

    animation_controller& play();
    animation_controller& pause();
    animation_controller& stop();
    animation_controller& time(const float ratio);
    animation_controller& loop(const bool enabled = true);
    animation_controller& speed(const float ratio);
    animation_controller& fade_in(const float duration = 0.1f); // trigger once then disappear
    animation_controller& fade_out(const float duration = 0.1f); // trigger once then disappear
    animation_controller& weight(const float ratio);

private:
    friend struct motion_system;
    bool _is_playing = false; // for testing
    bool _is_looping = true; // for testing
    float _playback_speed = 1.f;
    float _weight = 0.1f;
    float _computed_weight = 1.f;
    float _time_ratio = 0.f;
    std::optional<std::pair<float, float>> _fade_in_time_and_duration = std::nullopt;
    std::optional<std::pair<float, float>> _fade_out_time_and_duration = std::nullopt;
    float _last_time_ratio = 0.f;
    bool _has_looped = false;
};

struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    animator_component& animation(const unsigned int name, const std::shared_future<std::shared_ptr<animation_ref>>& fetched_animation);
    animator_component& animation(const unsigned int name, const std::shared_future<std::shared_ptr<animation_ref>>& fetched_animation, const std::shared_future<std::shared_ptr<motion_track_ref>>& fetched_motion_track);
    animator_component& motion_track(const unsigned int name, const std::shared_future<std::shared_ptr<motion_track_ref>>& fetched_motion_track);
    animator_component& skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton);
    animator_component& inverse_kinematics_chain(const unsigned int name, const std::string& start, const std::string& end);
    animator_component& inverse_kinematics_snap(const unsigned int name, const glm::vec3& end_position);
    

    animation_controller& get_controller(const unsigned int name);
    inline glm::mat4 get_bone_transform(const std::string& name)
    {
        if (_skeleton.has_value()) {
            const int num_joints = _skeleton.value().num_joints();
            for (int i = 0; i < num_joints; ++i) {
                if (std::string(_skeleton.value().joint_names()[i]) == name) {
                    return reinterpret(_model_transforms[i]);
                }
            }
        }        
        return glm::mat4(1.f);
    }

private:
    fetch_container<skeleton_ref> _skeleton = {};
    ozz::vector<ozz::math::SoaTransform> _blended_local_transforms = {};
    ozz::vector<ozz::math::Float4x4> _model_transforms = {};
    std::unordered_map<unsigned int, std::vector<std::reference_wrapper<transform_component>>> _children_transforms = {};
    std::unique_ptr<ozz::animation::SamplingJob::Context> _sampling_context = nullptr;
    std::unordered_map<unsigned int, animation_controller> _controllers = {};
    std::unordered_map<unsigned int, fetch_container<animation_ref>> _animations = {};
    std::unordered_map<unsigned int, fetch_container<motion_track_ref>> _motion_tracks = {};
    std::unordered_map<unsigned int, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
    ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers = {};
    friend struct transform_component;
    friend struct motion_system;
    friend struct rendering_system;
};