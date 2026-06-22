#include <cstring>

#include <woff2/decode.h>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_app.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/context_serialize.hpp>
#include <lucaria/engine/asset_font.hpp>

namespace lucaria {
namespace detail {

    object_font::object_font(manager_window& window, const std::vector<char>& bytes, const float32 font_size)
        : font_size(font_size)
    {
        int _data_size = static_cast<int>(bytes.size());
        void* _owned_data = IM_ALLOC(_data_size);
        std::memcpy(_owned_data, bytes.data(), _data_size);
        ImFontConfig _config;
        _config.FontDataOwnedByAtlas = true;
        font = window.shared_font_atlas->AddFontFromMemoryTTF(_owned_data, _data_size, font_size, &_config);
        LUCARIA_DEBUG_ASSERT(window.shared_font_atlas->Build(), "Failed to build ImGui font atlas")
        window.reupload_shared_imgui_font_texture();
    }

    void object_font::save(context_save_storage& context) const
    {
        context.field("origin_path", origin_path);
        const float32 _font_size = font != nullptr ? font->FontSize : font_size;
        context.field("font_size", _font_size);
    }

    void object_font::load(context_load_storage& context)
    {
        context.field("origin_path", origin_path);
        context.field("font_size", font_size);
        const std::filesystem::path _path = origin_path;
        const float32 _font_size = font_size;
        manager_window* _window = context.window();
        LUCARIA_DEBUG_ASSERT(_window != nullptr, "Failed to get manager_window while loading font asset")
        context.fetch(_path, [this, _path, _font_size, _window](const std::vector<char>& bytes) {
            const std::uint8_t* _raw_ptr = reinterpret_cast<const std::uint8_t*>(bytes.data());
            const std::size_t _expected_size = woff2::ComputeWOFF2FinalSize(_raw_ptr, bytes.size());
            LUCARIA_DEBUG_ASSERT(_expected_size > 0, "Failed to load woff2 font");
            std::string _output_str;
            _output_str.reserve(std::min(_expected_size, woff2::kDefaultMaxSize));
            woff2::WOFF2StringOut _woff2out(&_output_str);
            LUCARIA_DEBUG_ASSERT(woff2::ConvertWOFF2ToTTF(_raw_ptr, bytes.size(), &_woff2out), "Failed to decode woff2 font")
            std::vector<char> _decoded_bytes;
            _decoded_bytes.assign(std::make_move_iterator(_output_str.begin()), std::make_move_iterator(_output_str.end()));
            *this = object_font(*_window, _decoded_bytes, _font_size);
            origin_path = _path;
            font_size = _font_size;
        });
    }

}
}
