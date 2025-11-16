#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/semantics.hpp>

namespace lucaria {

/// @brief Represents a runtime font on the device
struct font {
    LUCARIA_DELETE_DEFAULT_SEMANTICS(font)
    font(const font& other) = delete;
    font& operator=(const font& other) = delete;
    font(font&& other) = default;
    font& operator=(font&& other) = default;

    /// @brief Creates a font from bytes synchronously
    /// @param data_bytes bytes to load from
    /// @param font_size imgui size of the font
    font(const std::vector<char>& data_bytes, const glm::float32 font_size);

    /// @brief Loads a font from a file synchronously
    /// @param data_path path to load from
    /// @param font_size imgui size of the font
    font(const std::filesystem::path& data_path, const glm::float32 font_size);

    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] ImFont* get_handle();
    
    /// @brief Returns a handle to the underlying implementation
    /// @return the underlying implementation handle
    [[nodiscard]] const ImFont* get_handle() const;

private:
    ImFont* _handle;
};

/// @brief Loads a font from a file asynchronously
/// @param data_path path to load from
/// @param font_size imgui size of the font
[[nodiscard]] fetched<font> fetch_font(const std::filesystem::path& data_path, const glm::float32 font_size);

}
