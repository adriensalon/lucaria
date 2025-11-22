#pragma once

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/track.h>

#include <lucaria/common/event_track_data.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/platform.hpp>

namespace lucaria {

/// @brief Represents a runtime animation
struct animation {
    LUCARIA_DELETE_DEFAULT(animation)
    animation(const animation& other) = delete;
    animation& operator=(const animation& other) = delete;
    animation(animation&& other) = default;
    animation& operator=(animation&& other) = default;

    /// @brief Loads an animation from bytes synchronously
    /// @param data_bytes bytes to load from
    animation(const std::vector<char>& data_bytes);

    /// @brief Loads an animation from a file synchronously
    /// @param data_path path to load from
    animation(const std::filesystem::path& data_path);

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] ozz::animation::Animation& get_handle();

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] const ozz::animation::Animation& get_handle() const;

private:
    ozz::animation::Animation _handle;
};

/// @brief Represents a runtime motion track
struct animation_motion_track {
    LUCARIA_DELETE_DEFAULT(animation_motion_track)
    animation_motion_track(const animation_motion_track& other) = delete;
    animation_motion_track& operator=(const animation_motion_track& other) = delete;
    animation_motion_track(animation_motion_track&& other) = default;
    animation_motion_track& operator=(animation_motion_track&& other) = default;

    /// @brief Loads a motion track from bytes synchronously
    /// @param data_bytes bytes to load from
    animation_motion_track(const std::vector<char>& data_bytes);

    /// @brief Loads a motion track from a file synchronously
    /// @param data_path path to load from
    animation_motion_track(const std::filesystem::path& data_path);

    /// @brief Returns a handle to the underlying implementation
    [[nodiscard]] ozz::animation::Float3Track& get_translation_handle();

    /// @brief Returns a handle to the underlying implementation
    [[nodiscard]] const ozz::animation::Float3Track& get_translation_handle() const;

    /// @brief Returns a handle to the underlying implementation
    [[nodiscard]] ozz::animation::QuaternionTrack& get_rotation_handle();

    /// @brief Returns a handle to the underlying implementation
    [[nodiscard]] const ozz::animation::QuaternionTrack& get_rotation_handle() const;

    /// @brief Returns a handle to the underlying implementation
    [[nodiscard]] glm::vec3 get_total_translation() const;

private:
    ozz::animation::Float3Track _translation_handle;
    ozz::animation::QuaternionTrack _rotation_handle;
};

/// @brief Represents a runtime event track
struct animation_event_track {
    LUCARIA_DELETE_DEFAULT(animation_event_track)
    animation_event_track(const animation_event_track& other) = delete;
    animation_event_track& operator=(const animation_event_track& other) = delete;
    animation_event_track(animation_event_track&& other) = default;
    animation_event_track& operator=(animation_event_track&& other) = default;

    /// @brief Loads a motion track from bytes synchronously
    /// @param data_bytes bytes to load from
    animation_event_track(const std::vector<char>& data_bytes);

    /// @brief Loads a motion track from a file synchronously
    /// @param data_path path to load from
    animation_event_track(const std::filesystem::path& data_path);

    event_track_data data;
};

/// @brief Loads an animation from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<animation> fetch_animation(const std::filesystem::path& data_path);

/// @brief Loads a motion track from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<animation_motion_track> fetch_motion_track(const std::filesystem::path& data_path);

/// @brief Loads an event track from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<animation_event_track> fetch_event_track(const std::filesystem::path& data_path);

}
