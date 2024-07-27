#pragma once

#include <future>
#include <memory>
#include <optional>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>

#include <core/animation.hpp>
#include <core/fetch.hpp>
#include <core/skeleton.hpp>

#include <ecs/component/transform.hpp>

struct animation_controller {
    bool is_playing = true;
    bool is_looping = true;
    glm::float32 time_ratio = 0.5f;
    glm::float32 playback_speed = 1.f;
};

struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    animator_component& animations(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& fetched_animations);
    animator_component& skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton);
    animator_component& motion_bone_index(const std::optional<glm::uint> bone_index);

    animation_controller& get_controller(const glm::uint name);

private:
    fetch_container<skeleton_ref> _skeleton = {};
    ozz::vector<ozz::math::Float4x4> _model_transforms = {};
    std::vector<std::vector<std::reference_wrapper<transform_component>>> _children_transforms = {};
    std::optional<glm::uint> _motion_bone_index = std::nullopt;
    std::unique_ptr<ozz::animation::SamplingJob::Context> _sampling_context = nullptr;
    std::unordered_map<glm::uint, animation_controller> _controllers = {};
    std::unordered_map<glm::uint, fetch_container<animation_ref>> _animations = {};
    std::unordered_map<glm::uint, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
    friend struct transform_component;
    friend struct motion_system;
    friend struct rendering_system;
};