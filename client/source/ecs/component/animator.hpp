#pragma once

#include <future>
#include <memory>
#include <optional>

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

    animator_component& armature(const std::shared_future<std::shared_ptr<armature_ref>>& fetched_armature);
    animator_component& skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton);
    animator_component& moveset(const std::shared_future<std::shared_ptr<moveset_ref>>& fetched_moveset);
    animator_component& play(const glm::uint& id);
    animator_component& pause(const glm::uint& id);
    animator_component& loop(const glm::uint& id, const bool must_loop);
    animator_component& warp(const glm::uint& id, const glm::float32 cursor_seconds);
    animator_component& weight(const glm::uint& id, const glm::float32 normalized);

private:
    fetch_container<armature_ref> _armature = {};
    fetch_container<skeleton_ref> _skeleton = {};
    fetch_container<moveset_ref> _moveset = {};
    friend struct motion_system;
};