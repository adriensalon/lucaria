#pragma once

#include <lucaria/common/shader_data.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/semantics.hpp>

namespace lucaria {

struct shader {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(shader)
    shader(const shader& other) = delete;
    shader& operator=(const shader& other) = delete;
    shader(shader&& other) = default;
    shader& operator=(shader&& other) = default;

    /// @brief Creates a shader from data
    /// @param data to create from
    shader(shader_data&& data);

    /// @brief Creates a shader from bytes synchronously
    /// @param image_bytes bytes to load from
    shader(const std::vector<char>& data_bytes);

    /// @brief Loads a shader from a file synchronously
    /// @param data_path path to load from
    shader(const std::filesystem::path& data_path);

    shader_data data;
};

/// @brief Loads a shader from a file asynchronously
/// @param data_path path to load from
[[nodiscard]] fetched<shader> fetch_shader(const std::filesystem::path data_path);

}
