#pragma once

#include <lucaria/bin/shader_data.hpp>
#include <lucaria/core/workaround.hpp>

namespace lucaria {

struct shader {
    LUCARIA_DELETE_DEFAULT(shader)
    shader(const shader& other) = delete;
    shader& operator=(const shader& other) = delete;
    shader(shader&& other) = default;
    shader& operator=(shader&& other) = default;

    shader(shader_data&& data);
    shader(const std::vector<char>& data_bytes);
    /// @brief $name
    void configure_field(const std::string& name, const std::string& value);
    /// @brief name< ... >name
    void configure_section(const std::string& name, const bool enable = true);

    shader_data data;
};

// /// @brief Loads a shader from a file asynchronously
// /// @param data_path path to load from
// [[nodiscard]] detail::async_container<shader> fetch_shader(const std::filesystem::path data_path);

}
