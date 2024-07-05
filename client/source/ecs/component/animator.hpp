#pragma once

#include <future>
#include <optional>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/animation_utils.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>

#include <core/animation.hpp>
#include <core/skeleton.hpp>

/// @brief
struct animator_component {
    animator_component() = default;
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

private:
    struct _animation_sampler {
        bool is_playing = false;
        bool is_looping = false;
        glm::float32 ratio = 0.f;
    };

    ozz::animation::SamplingJob _sampling_job = {};
    ozz::animation::LocalToModelJob _local_to_model_job = {};
    std::vector<glm::mat4> _model_matrices = {};
    std::optional<std::future<skeleton_ref>> _future_skeleton = std::nullopt;
    std::optional<skeleton_ref> _skeleton = std::nullopt;
    std::unordered_map<std::string, std::optional<std::future<animation_ref>>> _future_animations = {};
    std::unordered_map<std::string, std::optional<animation_ref>> _animations = {};
    std::unordered_map<std::string, _animation_sampler> _samplers = {};
    friend struct async_system;
    friend struct motion_system;
};