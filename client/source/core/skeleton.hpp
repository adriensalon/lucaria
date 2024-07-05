#pragma once

#include <filesystem>
#include <future>

#include <ozz/animation/runtime/skeleton.h>

/// @brief Represents an underlying ozz skeleton resource that can be loaded synchronously or not
/// from the filesystem.
struct skeleton_ref {
private:
    skeleton_ref() = default;
public:
    skeleton_ref(const skeleton_ref& other) = delete;
    skeleton_ref& operator=(const skeleton_ref& other) = delete;
    skeleton_ref(skeleton_ref&& other) = default;
    skeleton_ref& operator=(skeleton_ref&& other) = default;
    
    /// @brief Gets the underlying ozz skeleton.
    /// @return the ozz skeleton class instance.
    ozz::animation::Skeleton& get_skeleton();

private:
    ozz::animation::Skeleton _skeleton;
    friend skeleton_ref load_skeleton(const std::filesystem::path& file);
    friend std::future<skeleton_ref> fetch_skeleton(const std::filesystem::path& file);
};

/// @brief Loads a skeleton synchronously from a file.
/// @param file the file path to use.
/// @return the runtime skeleton_ref structure.
skeleton_ref load_skeleton(const std::filesystem::path& file);

/// @brief Loads a skeleton asynchronously from a file.
/// @param file the file path to use.
/// @return the runtime skeleton_ref structure.
std::future<skeleton_ref> fetch_skeleton(const std::filesystem::path& file);