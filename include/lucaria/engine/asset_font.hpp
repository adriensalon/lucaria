#pragma once

#include <imgui.h>

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>
#include <lucaria/engine/handle_asset.hpp>

namespace lucaria {
namespace detail {

    struct storage_save_context;
    struct storage_load_context;
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

        void save(storage_save_context& context) const;
        void load(storage_load_context& context);
    };

}

struct handle_font : handle_asset<detail::object_font> {
    using handle_asset<detail::object_font>::handle_asset;

    ImFont* imgui_font() const;
};

}
