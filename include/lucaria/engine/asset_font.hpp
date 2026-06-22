#pragma once

#include <imgui.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {

struct context_save_storage;
struct context_load_storage;

namespace detail {
    struct manager_window;

    struct object_font {
        object_font() = default;
        object_font(const object_font& other) = delete;
        object_font& operator=(const object_font& other) = delete;
        object_font(object_font&& other) = default;
        object_font& operator=(object_font&& other) = default;

        object_font(manager_window& window, const std::vector<char>& data_bytes, const float32 font_size);

        std::filesystem::path origin_path;
        ImFont* font = nullptr;
        float32 font_size = 14.f;

        void save(context_save_storage& context) const;
        void load(context_load_storage& context);
    };

}

struct handle_font : handle_asset<detail::object_font> {
    using handle_asset<detail::object_font>::handle_asset;

    ImFont* imgui_font() const;
};

}
