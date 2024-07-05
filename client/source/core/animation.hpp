#pragma once

#include <filesystem>
#include <future>

#include <ozz/animation/runtime/animation.h>

/// @brief Represents an underlying ozz animation resource that can be loaded synchronously or not
/// from the filesystem.
struct animation_ref {
private:
    animation_ref() = default;
public:
    animation_ref(const animation_ref& other) = delete;
    animation_ref& operator=(const animation_ref& other) = delete;
    animation_ref(animation_ref&& other) = default;
    animation_ref& operator=(animation_ref&& other) = default;

    /// @brief Gets the underlying ozz animation.
    /// @return the ozz animation class instance.
    ozz::animation::Animation& get_animation();

private:
    ozz::animation::Animation _animation;
    friend animation_ref load_animation(const std::filesystem::path& file);
    friend std::future<animation_ref> fetch_animation(const std::filesystem::path& file);
};

/// @brief Loads an animation synchronously from a file.
/// @param file the file path to use.
/// @return the runtime animation_ref structure.
animation_ref load_animation(const std::filesystem::path& file);

/// @brief Loads an animation asynchronously from a file.
/// @param file the file path to use.
/// @return the future to the runtime animation_ref structure.
std::future<animation_ref> fetch_animation(const std::filesystem::path& file);