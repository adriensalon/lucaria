#pragma once

#include <future>
#include <memory>
#include <optional>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>

#include <core/animation.hpp>
#include <core/fetch.hpp>
#include <core/skeleton.hpp>

#include <ecs/component/transform.hpp>

struct animation_controller {
    animation_controller() = default;
    animation_controller(const animation_controller& other) = delete;
    animation_controller& operator=(const animation_controller& other) = delete;
    animation_controller(animation_controller&& other) = default;
    animation_controller& operator=(animation_controller&& other) = default;

    animation_controller& play();
    animation_controller& pause();
    animation_controller& stop();
    animation_controller& time_warp(const glm::float32 ratio);
    animation_controller& time_relative(const glm::float32 ratio);
    animation_controller& loop(const bool enabled = true);
    animation_controller& speed(const glm::float32 ratio = 1.f);

private:
    friend struct animator_component;
    friend struct motion_system;
    bool _is_playing = true; // for testing
    bool _is_looping = true; // for testing
    glm::float32 _time_ratio = 0.f;
    glm::float32 _playback_speed = 1.f;
    bool _has_looped = false;
    glm::float32 _last_time_ratio = 0.f;
    fetch_container<animation_ref> _animation = {};
    // fetch_container<animation_track_motion_ref> _motion = {};
    ozz::vector<ozz::math::SoaTransform> _local_transforms = {};
};

struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    animator_component& animations(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& fetched_animations);
    animator_component& skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton);
    animator_component& motion_bone(const std::optional<glm::uint>& bone_index);
    animator_component& motion_bone(const std::optional<std::string>& bone_name);

    animation_controller& get_controller(const glm::uint name);

private:
    fetch_container<skeleton_ref> _skeleton = {};
    ozz::vector<ozz::math::SoaTransform> _blended_local_transforms = {};
    ozz::vector<ozz::math::Float4x4> _model_transforms = {};
    std::vector<std::vector<std::reference_wrapper<transform_component>>> _children_transforms = {};
    std::optional<std::string> _motion_bone_name = std::nullopt;
    std::optional<glm::uint> _motion_bone_index = std::nullopt;    
    std::unique_ptr<ozz::animation::SamplingJob::Context> _sampling_context = nullptr;
    std::unordered_map<glm::uint, animation_controller> _controllers = {};

    // to remove
    bool _just_started = false;
    ozz::vector<ozz::math::Float4x4> _model_transforms_copy = {};
    ozz::vector<ozz::math::Float4x4> _model_output_transforms = {};
    ozz::vector<ozz::math::Float4x4> _model_last_transforms = {};
    std::optional<glm::mat4> _motion_last_transform = std::nullopt;
    std::optional<glm::mat4> _motion_last_transform_copy = std::nullopt;
    glm::mat4 _accumulated = glm::mat4(1.f);
    std::unordered_map<glm::uint, fetch_container<animation_ref>> _animations = {};
    std::unordered_map<glm::uint, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
    friend struct transform_component;
    friend struct motion_system;
    friend struct rendering_system;
};