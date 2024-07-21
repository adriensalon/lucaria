#pragma once

#include <future>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/containers/vector.h>

#include <core/animation.hpp>
#include <core/armature.hpp>
#include <core/fetch.hpp>
#include <core/skeleton.hpp>

struct animation_controller {
    bool is_playing = true;
    bool is_looping = true;
    glm::float32 time_ratio = 0.f;
    glm::float32 playback_speed = 1.f;
};

struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    animator_component& animations(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& fetched_animations);
    animator_component& armature(const std::shared_future<std::shared_ptr<armature_ref>>& fetched_armature);
    animator_component& skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton);

    animation_controller& get_controller(const glm::uint name);

private:
    fetch_container<armature_ref> _armature = {};
    fetch_container<skeleton_ref> _skeleton = {};
    ozz::vector<ozz::math::Float4x4> _model_transforms = {};
    std::unique_ptr<ozz::animation::SamplingJob::Context> _sampling_context = nullptr;
    std::unordered_map<glm::uint, animation_controller> _controllers = {};
    std::unordered_map<glm::uint, fetch_container<animation_ref>> _animations = {};
    std::unordered_map<glm::uint, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
    friend struct motion_system;
};