#pragma once

#include <filesystem>
#include <future>
#include <memory>
#include <unordered_map>

#include <ozz/animation/runtime/blending_job.h>

#include <core/animation.hpp>

struct moveset_ref {
    moveset_ref() = delete;
    moveset_ref(const moveset_ref& other) = delete;
    moveset_ref& operator=(const moveset_ref& other) = delete;
    moveset_ref(moveset_ref&& other) = default;
    moveset_ref& operator=(moveset_ref&& other) = default;
    
    moveset_ref(const std::unordered_map<glm::uint, animation_data>& animations);
    animation_ref& get_animation(const glm::uint& id);
    ozz::animation::BlendingJob& get_job();

private:
    std::unordered_map<glm::uint, std::shared_ptr<animation_ref>> _animations;
    ozz::animation::BlendingJob _blending_job;
};

std::shared_future<std::shared_ptr<moveset_ref>> fetch_moveset(const std::unordered_map<glm::uint, std::filesystem::path>& animation_paths);
