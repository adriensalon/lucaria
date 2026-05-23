#pragma once

#include <imgui.h>

#include <lucaria/core/image.hpp>

#if defined(LUCARIA_BACKEND_OPENGL)
#include <lucaria/core/backend/opengl/texture_opengl.hpp>
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
#include <lucaria/core/backend/pspgu/texture_pspgu.hpp>
#endif

namespace lucaria {

struct texture_object;

namespace detail {

    struct rendering_system;
    struct implementation_save_database;

    enum struct texture_origin {
        path,
        data,
        size
    };

    struct texture_implementation {
        LUCARIA_DELETE_DEFAULT(texture_implementation)
        texture_implementation(const texture_implementation& other) = delete;
        texture_implementation& operator=(const texture_implementation& other) = delete;
        texture_implementation(texture_implementation&& other) = default;
        texture_implementation& operator=(texture_implementation&& other) = default;
        ~texture_implementation();

        texture_implementation(const image_implementation& image);
        texture_implementation(const uint32x2 size);
        void resize(const uint32x2 new_size);
        void update(const image_implementation& image);
        [[nodiscard]] ImTextureID imgui_texture() const;

        texture_origin origin;
        uint32x2 size;

#if defined(LUCARIA_BACKEND_OPENGL)
        texture_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        texture_implementation_pspgu implementation_pspgu;
#endif
    };

    struct texture_path_recipe {
        std::filesystem::path path;
        std::optional<std::filesystem::path> etc2_path;
        std::optional<std::filesystem::path> s3tc_path;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
            archive(cereal::make_nvp("etc2_path", etc2_path));
            archive(cereal::make_nvp("s3tc_path", s3tc_path));
        }
    };

    struct texture_data_recipe {
        image_data data;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("data", data));
        }
    };

    struct texture_size_recipe {
        uint32x2 size;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("size", size));
        }
    };

    using texture_recipe = std::variant<texture_path_recipe, texture_data_recipe, texture_size_recipe>;

    [[nodiscard]] texture_recipe make_recipe(const implementation_container<texture_implementation>& container);
   
    // [[nodiscard]] resource_container<texture_implementation>& apply_recipe(implementation_manager<texture_implementation>& manager, texture_recipe&& recipe);

}

/// @brief Represents a texture on the device. Can be created from an image file or from an empty size.
struct texture_object {
    texture_object() = default;
    texture_object(const texture_object& other) = default;
    texture_object& operator=(const texture_object& other) = default;
    texture_object(texture_object&& other) = default;
    texture_object& operator=(texture_object&& other) = default;
    ~texture_object();

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
    detail::refcount_flag _refcount = {};
    detail::implementation_manager<detail::texture_implementation>* _manager = nullptr;
    detail::implementation_container<detail::texture_implementation>* _resource = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

    friend struct detail::rendering_system;
	friend class cereal::access;
};

}
