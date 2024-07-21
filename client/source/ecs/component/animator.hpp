#pragma once

#include <future>
#include <memory>
#include <optional>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/animation_utils.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/simd_math.h>

#include <core/armature.hpp>
#include <core/moveset.hpp>
#include <core/skeleton.hpp>
#include <core/fetch.hpp>

struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    animator_component& animations(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& fetched_animations);
    animator_component& armature(const std::shared_future<std::shared_ptr<armature_ref>>& fetched_armature);
    animator_component& skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton);
    
    animation_ref& get_animation(const glm::uint name);

private:
    std::unordered_map<glm::uint, fetch_container<animation_ref>> _animations = {};
    fetch_container<armature_ref> _armature = {};
    fetch_container<skeleton_ref> _skeleton = {};
    friend struct motion_system;
};