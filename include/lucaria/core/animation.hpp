#pragma once

#include <ozz/animation/runtime/animation.h>

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/semantics.hpp>

namespace lucaria {

/// @brief Represents a runtime animation
struct animation {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(animation)
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

/// @brief Loads an animation from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<animation> fetch_animation(const std::filesystem::path& data_path);

}
