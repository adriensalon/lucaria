#pragma once

#include <lucaria/common/geometry_data.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/semantics.hpp>

namespace lucaria {

/// @brief Represents any geometry on the host
struct geometry {
    geometry() = default;
    geometry(const geometry& other) = delete;
    geometry& operator=(const geometry& other) = delete;
    geometry(geometry&& other) = default;
    geometry& operator=(geometry&& other) = default;

    /// @brief Creates geometry from bytes synchronously
    /// @param data_bytes bytes to load from
    geometry(const std::vector<char>& data_bytes);

    /// @brief Loads geometry from a file synchronously
    /// @param data_path path to load from
    geometry(const std::filesystem::path& data_path);

    geometry_data data;
};

/// @brief Loads geometry from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<geometry> fetch_geometry(const std::filesystem::path& data_path);

}
