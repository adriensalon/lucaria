#include <cstring>

#include <woff2/decode.h>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/manager_window.hpp>
#include <lucaria/core/object_font.hpp>
#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    std::vector<char> decode_font_bytes(const std::vector<char>& bytes)
    {
        const std::uint8_t* _raw_ptr = reinterpret_cast<const std::uint8_t*>(bytes.data());
        const std::size_t _expected_size = woff2::ComputeWOFF2FinalSize(_raw_ptr, bytes.size());
        if (_expected_size == 0) {
            return bytes;
        }

        std::string _output_str;
        _output_str.reserve(std::min(_expected_size, woff2::kDefaultMaxSize));
        woff2::WOFF2StringOut _woff2out(&_output_str);
        if (!woff2::ConvertWOFF2ToTTF(_raw_ptr, bytes.size(), &_woff2out)) {
            LUCARIA_DEBUG_ERROR("Impossible to decode woff2 font")
        }

        std::vector<char> _output;
        _output.assign(std::make_move_iterator(_output_str.begin()), std::make_move_iterator(_output_str.end()));
        return _output;
    }

    object_font::object_font(manager_window& window, const std::vector<char>& bytes, const float32 font_size)
        : origin(object_font_origin::path)
        , font_size(font_size)
    {
        int _data_size = static_cast<int>(bytes.size());
        void* _owned_data = IM_ALLOC(_data_size);
        std::memcpy(_owned_data, bytes.data(), _data_size);
        ImFontConfig _config;
        _config.FontDataOwnedByAtlas = true;
        font = window.shared_font_atlas->AddFontFromMemoryTTF(_owned_data, _data_size, font_size, &_config);
        if (!window.shared_font_atlas->Build()) {
            LUCARIA_DEBUG_ERROR("Failed to build ImGui font atlas")
        }
        window.reupload_shared_imgui_font_texture();
    }

}
}
