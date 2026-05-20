#pragma once

#include <imgui.h>

#include <lucaria/core/image.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/resource.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend/opengl/texture_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend/pspgu/texture_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

	struct rendering_system;

    struct texture_implementation {
        LUCARIA_DELETE_DEFAULT(texture_implementation)
        texture_implementation(const texture_implementation& other) = delete;
        texture_implementation& operator=(const texture_implementation& other) = delete;
        texture_implementation(texture_implementation&& other);
        texture_implementation& operator=(texture_implementation&& other);
        ~texture_implementation();

        texture_implementation(const image_implementation& image);
        texture_implementation(const uint32x2 size);
        void resize(const uint32x2 new_size);
        void update(const image_implementation& image);
        [[nodiscard]] ImTextureID imgui_texture() const;

#if defined(LUCARIA_BACKEND_OPENGL)
        texture_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        texture_implementation_pspgu implementation_pspgu;
#endif

        uint32x2 size;
    };

}

/// @brief Represents a texture on the device. Can be created from an image file or from an empty size.
struct texture_object {
    texture_object() = default;
    texture_object(const texture_object& other) = default;
    texture_object& operator=(const texture_object& other) = default;
    texture_object(texture_object&& other) = default;
    texture_object& operator=(texture_object&& other) = default;

    /// TODO GO CONTEXT
    static texture_object create(const uint32x2 size);

    /// TODO GO CONTEXT
    static texture_object fetch(
        const std::filesystem::path& path,
        const std::optional<std::filesystem::path>& etc2_path = std::nullopt,
        const std::optional<std::filesystem::path>& s3tc_path = std::nullopt);

    /// @brief Checks if the texture is ready to be used
    /// @return true if the texture is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

    /// @brief Resizes the texture to the specified size
    /// @param size the new size of the texture
    void resize(const uint32x2 new_size);

    /// @brief Gets the size of the texture
    /// @return the size of the texture
    [[nodiscard]] uint32x2 size() const;

    [[nodiscard]] ImTextureID imgui_texture() const;

private:
    detail::resource_container<detail::texture_implementation>* _resource = nullptr;
    explicit texture_object(detail::resource_container<detail::texture_implementation>* resource);
    friend struct detail::rendering_system;
};

}
