#pragma once

#include <cereal/types/array.hpp>
#include <cereal/types/optional.hpp>

#include <lucaria/core/image.hpp>
#include <lucaria/core/refcount.hpp>
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

    enum struct cubemap_origin {
        path,
        data
    };

    struct cubemap_implementation {
        LUCARIA_DELETE_DEFAULT(cubemap_implementation)
        cubemap_implementation(const cubemap_implementation& other) = delete;
        cubemap_implementation& operator=(const cubemap_implementation& other) = delete;
        cubemap_implementation(cubemap_implementation&& other) = default;
        cubemap_implementation& operator=(cubemap_implementation&& other) = default;
        ~cubemap_implementation();

        cubemap_implementation(const std::array<image_implementation, 6>& images);

        cubemap_origin origin;

#if defined(LUCARIA_BACKEND_OPENGL)
        cubemap_implementation_opengl implementation_opengl;
#endif

#if defined(LUCARIA_BACKEND_PSPGU)
        cubemap_implementation_pspgu implementation_pspgu;
#endif
    };

    struct cubemap_path_recipe {
        std::array<std::filesystem::path, 6> paths;
        std::optional<std::array<std::filesystem::path, 6>> etc2_paths;
        std::optional<std::array<std::filesystem::path, 6>> s3tc_paths;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("paths", paths));
            archive(cereal::make_nvp("etc2_paths", etc2_paths));
            archive(cereal::make_nvp("s3tc_paths", s3tc_paths));
        }
    };

    struct cubemap_data_recipe {
        std::array<image_data, 6> datas;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("datas", datas));
        }
    };

    using cubemap_recipe = std::variant<cubemap_path_recipe, cubemap_data_recipe>;

    [[nodiscard]] cubemap_recipe make_recipe(const implementation_container<cubemap_implementation>& container);
}

struct cubemap_object {
    cubemap_object() = default;
    cubemap_object(const cubemap_object& other) = default;
    cubemap_object& operator=(const cubemap_object& other) = default;
    cubemap_object(cubemap_object&& other) = default;
    cubemap_object& operator=(cubemap_object&& other) = default;
    ~cubemap_object();

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
    detail::refcount_flag _refcount = {};
    detail::implementation_manager<detail::cubemap_implementation>* _manager = nullptr;
    detail::implementation_container<detail::cubemap_implementation>* _resource = nullptr;
    
	template <typename ArchiveType>
    void save(ArchiveType& archive) const;
    template <typename ArchiveType>
    void load(ArchiveType& archive);

    friend struct detail::rendering_system;
	friend class cereal::access;
};

}
