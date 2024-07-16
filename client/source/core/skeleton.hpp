#pragma once

#include <filesystem>
#include <future>
#include <memory>

#include <glm/glm.hpp>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/maths/simd_math.h>

using skeleton_data = std::shared_ptr<ozz::animation::Skeleton>;

struct skeleton_ref {
    skeleton_ref() = delete;
    skeleton_ref(const skeleton_ref& other) = delete;
    skeleton_ref& operator=(const skeleton_ref& other) = delete;
    skeleton_ref(skeleton_ref&& other) = default;
    skeleton_ref& operator=(skeleton_ref&& other) = default;

    skeleton_ref(const skeleton_data& data);
    ozz::animation::LocalToModelJob& get_job();
    glm::uint get_transforms_size() const;
    glm::uint get_soa_transforms_size() const;

private:
    ozz::animation::Skeleton _skeleton;
    ozz::animation::LocalToModelJob _local_to_model_job;
};

skeleton_data load_skeleton_data(std::istringstream& skeleton_stream);
std::shared_future<std::shared_ptr<skeleton_ref>> fetch_skeleton(const std::filesystem::path& skeleton_path);
