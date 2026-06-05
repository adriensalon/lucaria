#pragma once

#include <imgui.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>
#include <lucaria/core/assets_buffer.hpp>
#include <lucaria/core/serialize_context.hpp>
#include <lucaria/forward/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;

    struct manager_assets;
    struct manager_window;

    [[nodiscard]] std::vector<char> decode_font_bytes(const std::vector<char>& bytes);

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

        void save(storage_save_context& context) const
        {
            context.field("origin", origin);
            context.field("origin_path", origin_path);
            const float32 _font_size = font != nullptr ? font->FontSize : font_size;
            context.field("font_size", _font_size);
        }

        void load(storage_load_context& context)
        {
            context.field("origin", origin);
            context.field("origin_path", origin_path);
            context.field("font_size", font_size);
            const std::filesystem::path _path = origin_path;
            const float32 _font_size = font_size;
            manager_window* _window = context.window();
            if (_window == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_window while loading font asset");
                return;
            }
            context.fetch(_path, [this, _path, _font_size, _window](const std::vector<char>& bytes) {
                std::vector<char> _font_bytes = decode_font_bytes(bytes);
                *this = object_font(*_window, _font_bytes, _font_size);
                origin_path = _path;
                font_size = _font_size;
            });
        }
    };

}

struct handle_font : handle_asset<detail::object_font> {
    using handle_asset<detail::object_font>::handle_asset;

    ImFont* imgui_font() const;
};

}
