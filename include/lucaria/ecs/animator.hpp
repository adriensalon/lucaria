#pragma once

#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/ecs/transform.hpp>

namespace lucaria {

/// @brief
struct animation_controller {
    animation_controller() = default;
    animation_controller(const animation_controller& other) = delete;
    animation_controller& operator=(const animation_controller& other) = delete;
    animation_controller(animation_controller&& other) = default;
    animation_controller& operator=(animation_controller&& other) = default;

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_play();

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_pause();

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_stop();

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_time(const glm::float32 ratio);

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_loop(const bool enable = true);

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_speed(const glm::float32 ratio);

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_weight(const glm::float32 ratio);

    /// @brief
    /// @param name
    /// @param callback
    /// @return this instance for chaining methods
    animation_controller& set_event_callback(const std::string& name, const std::function<void()>& callback);

private:
    bool _is_playing = false;
    bool _is_looping = true;
    glm::float32 _playback_speed = 1.f;
    glm::float32 _weight = 1.f;
    glm::float32 _time_ratio = 0.f;
    glm::float32 _last_time_ratio = 0.f;
    bool _has_looped = false;
    std::unordered_map<std::string, std::function<void()>> _event_callbacks = {};
    friend struct motion_system;
    friend struct animator_component;
};

/// @brief
struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_animation(const std::string name, animation& from);

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_animation(const std::string name, fetched<animation>& from);

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_motion_track(const std::string name, animation_motion_track& from);

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_motion_track(const std::string name, fetched<animation_motion_track>& from);

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_event_track(const std::string name, animation_event_track& from);

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_event_track(const std::string name, fetched<animation_event_track>& from);

    /// @brief
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_skeleton(skeleton& from);

    /// @brief
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_skeleton(fetched<skeleton>& from);

    // animator_component& use_inverse_kinematics_chain(const std::string name, const std::string& start, const std::string& end);
    // animator_component& use_inverse_kinematics_snap(const std::string name, const glm::vec3& end_position);

    /// @brief
    /// @param name
    /// @return
    [[nodiscard]] animation_controller& get_controller(const std::string& name);

    /// @brief
    /// @param bone
    /// @return
    [[nodiscard]] glm::mat4 get_bone_transform(const std::string& bone);

private:
    _detail::fetched_container<skeleton> _skeleton = {};
    std::unordered_map<std::string, _detail::fetched_container<animation>> _animations = {};
    std::unordered_map<std::string, _detail::fetched_container<animation_motion_track>> _motion_tracks = {};
    std::unordered_map<std::string, _detail::fetched_container<animation_event_track>> _event_tracks = {};
    std::unordered_map<std::string, std::vector<std::reference_wrapper<transform_component>>> _children_transforms = {};
    std::unordered_map<std::string, animation_controller> _controllers = {};
    std::unordered_map<std::string, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
    ozz::vector<ozz::math::SoaTransform> _blended_local_transforms = {};
    ozz::vector<ozz::math::Float4x4> _model_transforms = {};
    std::unique_ptr<ozz::animation::SamplingJob::Context> _sampling_context = nullptr;
    ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers = {};
    friend struct transform_component;
    friend struct motion_system;
    friend struct rendering_system;
};

}
