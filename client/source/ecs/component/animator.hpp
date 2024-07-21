#pragma once

#include <future>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>
#include <ozz/base/maths/simd_math.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/containers/vector.h>

#include <core/animation.hpp>
#include <core/armature.hpp>
#include <core/fetch.hpp>
#include <core/skeleton.hpp>

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
    fetch_container<armature_ref> _armature = {};
    fetch_container<skeleton_ref> _skeleton = {};
    std::unordered_map<glm::uint, fetch_container<animation_ref>> _animations = {};
    std::unordered_map<glm::uint, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
    ozz::vector<ozz::math::Float4x4> _model_transforms = {};
    friend struct motion_system;
};