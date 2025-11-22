#pragma once

#include <ozz/animation/runtime/skeleton.h>

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/platform.hpp>

namespace lucaria {

/// @brief Represents a runtime skeleton
struct skeleton {
    LUCARIA_DELETE_DEFAULT(skeleton)
    skeleton(const skeleton& other) = delete;
    skeleton& operator=(const skeleton& other) = delete;
    skeleton(skeleton&& other) = default;
    skeleton& operator=(skeleton&& other) = default;

    /// @brief Loads a skeleton from bytes synchronously
    /// @param data_bytes bytes to load from
    skeleton(const std::vector<char>& data_bytes);

    /// @brief Loads a skeleton from a file synchronously
    /// @param data_path path to load from
    skeleton(const std::filesystem::path& data_path);

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] ozz::animation::Skeleton& get_handle();
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] const ozz::animation::Skeleton& get_handle() const;

private:
    ozz::animation::Skeleton _handle;
};

/// @brief Loads a skeleton from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<skeleton> fetch_skeleton(const std::filesystem::path& data_path);

}
