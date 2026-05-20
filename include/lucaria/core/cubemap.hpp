#pragma once

#include <lucaria/core/image.hpp>
#include <lucaria/core/resource.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend/opengl/cubemap_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend/pspgu/cubemap_pspgu.hpp>
#endif

namespace lucaria {
namespace detail {

	struct rendering_system;

    struct cubemap_implementation {
        LUCARIA_DELETE_DEFAULT(cubemap_implementation)
        cubemap_implementation(const cubemap_implementation& other) = delete;
        cubemap_implementation& operator=(const cubemap_implementation& other) = delete;
        cubemap_implementation(cubemap_implementation&& other);
        cubemap_implementation& operator=(cubemap_implementation&& other);
        ~cubemap_implementation();

        cubemap_implementation(const std::array<image_implementation, 6>& images);

#if defined(LUCARIA_BACKEND_OPENGL)
        cubemap_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        cubemap_implementation_pspgu implementation_pspgu;
#endif
    };

}

struct cubemap_object {
    cubemap_object() = default;
    cubemap_object(const cubemap_object& other) = default;
    cubemap_object& operator=(const cubemap_object& other) = default;
    cubemap_object(cubemap_object&& other) = default;
    cubemap_object& operator=(cubemap_object&& other) = default;

    /// TODO GO CONTEXT
    static cubemap_object create(const glm::uvec2 size);

    /// TODO GO CONTEXT
    static cubemap_object fetch(
        const std::array<std::filesystem::path, 6>& data_paths,
        const std::optional<std::array<std::filesystem::path, 6>>& etc2_paths = std::nullopt,
        const std::optional<std::array<std::filesystem::path, 6>>& s3tc_paths = std::nullopt);

    /// @brief Checks if the cubemap is ready to be used
    /// @return true if the cubemap is ready, false otherwise
    [[nodiscard]] bool has_value() const;

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const;

private:
    detail::resource_container<detail::cubemap_implementation>* _resource = nullptr;
    explicit cubemap_object(detail::resource_container<detail::cubemap_implementation>* resource);
    friend struct detail::rendering_system;
};

}
