#pragma once

#include <future>
#include <optional>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/animation_utils.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/simd_math.h>

#include <core/animation.hpp>
#include <core/skeleton.hpp>

/// @brief 
struct animation_sampler_ref {
    animation_sampler_ref();
    animation_sampler_ref(const animation_sampler_ref& other) = delete;
    animation_sampler_ref& operator=(const animation_sampler_ref& other) = delete;
    animation_sampler_ref(animation_sampler_ref&& other) = default;
    animation_sampler_ref& operator=(animation_sampler_ref&& other) = default;

    bool is_prepared;
    bool is_playing;
    bool is_looping;
    glm::float32 ratio;
    glm::float32 weight;
    ozz::animation::SamplingJob sampling_job;
    std::vector<ozz::math::SoaTransform> local_transforms;
};

/// @brief
struct animator_component {
    animator_component();
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    /// @brief
    /// @param name
    /// @param value
    animator_component& animation(const std::string& name, animation_ref&& value);

    /// @brief
    /// @param name
    /// @param value
    animator_component& animation(const std::string& name, std::future<animation_ref>&& value);

    /// @brief
    /// @param value
    animator_component& skeleton(skeleton_ref&& value);

    /// @brief
    /// @param value
    animator_component& skeleton(std::future<skeleton_ref>&& value);

    /// @brief
    /// @param name
    animator_component& play(const std::string& name);

    /// @brief
    /// @param name
    animator_component& pause(const std::string& name);

    /// @brief
    /// @param name
    /// @param value
    animator_component& loop(const std::string& name, const bool value);

    /// @brief
    /// @param name
    /// @param value
    animator_component& warp(const std::string& name, const glm::float32 value);

    /// @brief
    /// @param name
    /// @param value
    animator_component& weight(const std::string& name, const glm::float32 value);

private:
    bool _is_prepared;
    bool _is_bound_to_model;
    std::optional<std::future<skeleton_ref>> _future_skeleton;
    std::optional<skeleton_ref> _skeleton;
    std::unordered_map<std::string, std::optional<std::future<animation_ref>>> _future_animations;
    std::unordered_map<std::string, std::optional<animation_ref>> _animations;
    std::unordered_map<std::string, animation_sampler_ref> _samplers;
    std::vector<ozz::math::SoaTransform> _blended_local_transforms;
    ozz::animation::LocalToModelJob _local_to_model_job;
    std::vector<ozz::math::Float4x4> _model_transforms;
    friend struct async_system;
    friend struct motion_system;
};