#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <sstream>

#include <glm/glm.hpp>

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/soa_transform.h>

using animation_data = std::shared_ptr<ozz::animation::Animation>;

struct animation_ref {
    animation_ref() = delete;
    animation_ref(const animation_ref& other) = delete;
    animation_ref& operator=(const animation_ref& other) = delete;
    animation_ref(animation_ref&& other) = default;
    animation_ref& operator=(animation_ref&& other) = default;

    animation_ref(const animation_data& data);
    float get_duration() const;
    ozz::animation::SamplingJob& get_job();
    // animator_component& play(const glm::uint& id);
    // animator_component& pause(const glm::uint& id);
    // animator_component& loop(const glm::uint& id, const bool must_loop);
    // animator_component& warp(const glm::uint& id, const glm::float32 cursor_seconds);
    // animator_component& weight(const glm::uint& id, const glm::float32 normalized);

    
    bool is_playing = false;
    bool is_looping = false;
    glm::float32 cursor = 0.f;
    glm::float32 weight = 1.f;

private:
    ozz::animation::Animation _animation;
    ozz::animation::SamplingJob _sampling_job;
};

animation_data load_animation_data(std::istringstream& animation_stream);
std::shared_future<std::shared_ptr<animation_ref>> fetch_animation(const std::filesystem::path& animation_path);
