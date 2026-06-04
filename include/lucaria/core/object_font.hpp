#pragma once

#include <imgui.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/utils_compiler.hpp>
#include <lucaria/core/utils_refcount.hpp>

namespace lucaria {
namespace detail {

    struct manager_assets;
    struct manager_window;

    enum struct object_font_origin {
        path
    };

    struct object_font {
        object_font() = default;
        object_font(const object_font& other) = delete;
        object_font& operator=(const object_font& other) = delete;
        object_font(object_font&& other) = default;
        object_font& operator=(object_font&& other) = default;

        object_font(manager_window& window, const std::vector<char>& data_bytes, const float32 font_size);

        object_font_origin origin;
        std::filesystem::path origin_path;
        ImFont* font = nullptr;
        float32 font_size = 14.f;

        template <typename ContextType>
        void save(ContextType& context) const
        {
            context(cereal::make_nvp("origin", origin));
            context(cereal::make_nvp("origin_path", origin_path));
            const float32 _font_size = font != nullptr ? font->FontSize : font_size;
            context(cereal::make_nvp("font_size", _font_size));
        }

        template <typename ContextType>
        void load(ContextType& context)
        {
            context(cereal::make_nvp("origin", origin));
            context(cereal::make_nvp("origin_path", origin_path));
            context(cereal::make_nvp("font_size", font_size));
            const std::filesystem::path _path = origin_path;
            const float32 _font_size = font_size;
            manager_window* _window = context.window();
            if (_window == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_window while loading font asset");
                return;
            }
            context.fetch(_path, [this, _path, _font_size, _window](const std::vector<char>& bytes) {
                *this = object_font(*_window, bytes, _font_size);
                origin_path = _path;
                font_size = _font_size;
            });
        }

    };

    [[nodiscard]] assets_cell<object_font>& fetch(
        manager_window& window,
        manager_assets& objects,
        assets_buffer<object_font>& cached_vector,
        const std::filesystem::path& path,
        const float32 font_size);

	//
	//
	//
	//
	//
	//
	//
	//
	//
    // recipes

    struct recipe_object_font_path {
        std::filesystem::path path;
        float32 font_size;

        template <typename ArchiveType>
        void serialize(ArchiveType& archive)
        {
            archive(cereal::make_nvp("path", path));
            archive(cereal::make_nvp("font_size", font_size));
        }
    };

    using recipe_object_font = std::variant<recipe_object_font_path>;

    [[nodiscard]] recipe_object_font make_recipe(const assets_cell<object_font>& cache);
    [[nodiscard]] assets_cell<object_font>* apply_recipe(manager_window& window, manager_assets& objects, assets_buffer<object_font>& cached, recipe_object_font& recipe);

}
}
