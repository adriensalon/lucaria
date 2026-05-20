#pragma once

#include <imgui.h>

#include <lucaria/core/math.hpp>
#include <lucaria/core/refcount.hpp>
#include <lucaria/core/resource.hpp>
#include <lucaria/core/workaround.hpp>

namespace lucaria {
namespace detail {

    struct font_implementation {
        LUCARIA_DELETE_DEFAULT(font_implementation)
        font_implementation(const font_implementation& other) = delete;
        font_implementation& operator=(const font_implementation& other) = delete;
        font_implementation(font_implementation&& other) = default;
        font_implementation& operator=(font_implementation&& other) = default;

        font_implementation(const std::vector<char>& data_bytes, const float32 font_size);

        ImFont* font;
    };

}

struct font_object {
    font_object() = default;
    font_object(const font_object& other) = default;
    font_object& operator=(const font_object& other) = default;
    font_object(font_object&& other) = default;
    font_object& operator=(font_object&& other) = default;
    ~font_object();

    static font_object fetch(const std::filesystem::path& path, const float32 font_size);

    /// @brief Checks if the font is ready to be used
    /// @return true if the font is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

    [[nodiscard]] ImFont* imgui_font() const;

private:
    detail::refcount_flag _refcount = {};
    detail::resource_manager<detail::font_implementation>* _manager = nullptr;
    detail::resource_container<detail::font_implementation>* _resource = nullptr;
    explicit font_object(detail::resource_container<detail::font_implementation>* resource);
};

}
