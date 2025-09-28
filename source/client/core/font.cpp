#include <iostream>

#include <backends/imgui_impl_opengl3.h>
#include <woff2/decode.h>

#include <lucaria/core/error.hpp>
#include <lucaria/core/font.hpp>
#include <lucaria/core/hash.hpp>
#include <lucaria/core/window.hpp>

namespace lucaria {
namespace {

    ImFont* load_data_from_bytes(const std::vector<char>& data_bytes, const glm::float32 font_size)
    {
        int _data_size = static_cast<int>(data_bytes.size());
        void* _owned_data = IM_ALLOC(_data_size);
        memcpy(_owned_data, data_bytes.data(), _data_size);

        ImFontConfig _config;
        _config.FontDataOwnedByAtlas = true;

        ImFont* _font = detail::imgui_shared_font_atlas->AddFontFromMemoryTTF(_owned_data, _data_size, font_size, &_config);
        if (!detail::imgui_shared_font_atlas->Build()) {
            LUCARIA_RUNTIME_ERROR("Failed to build ImGui font atlas")
        }
        // ImGui_ImplOpenGL3_DestroyFontsTexture();
        // ImGui_ImplOpenGL3_CreateFontsTexture();

        detail::reupload_shared_font_texture_RGBA32();
        return _font;
    }

}

font::font(const std::vector<char>& data_bytes, const glm::float32 font_size)
{
    _handle = load_data_from_bytes(data_bytes, font_size);
}

font::font(const std::filesystem::path& data_path, const glm::float32 font_size)
{
    detail::load_bytes(data_path, [this, font_size](const std::vector<char>& _data_bytes) {
        _handle = load_data_from_bytes(_data_bytes, font_size);
    });
}

ImFont* font::get_handle()
{
    return _handle;
}

const ImFont* font::get_handle() const
{
    return _handle;
}

fetched<font> fetch_font(const std::filesystem::path& data_path, const glm::float32 font_size)
{
    std::shared_ptr<std::promise<std::shared_ptr<std::vector<char>>>> _data_promise = std::make_shared<std::promise<std::shared_ptr<std::vector<char>>>>();

    detail::fetch_bytes(data_path, [_data_promise](const std::vector<char>& _data_bytes) {
        const std::uint8_t* _raw_ptr = reinterpret_cast<const uint8_t*>(_data_bytes.data());
        const std::size_t _expected_size = woff2::ComputeWOFF2FinalSize(_raw_ptr, _data_bytes.size());
        if (_expected_size == 0) {
            LUCARIA_RUNTIME_ERROR("Failed to compute woff2 final size");
        }

        std::string _output_str;
        _output_str.reserve(std::min(_expected_size, woff2::kDefaultMaxSize));

        woff2::WOFF2StringOut _woff2out(&_output_str);
        if (!woff2::ConvertWOFF2ToTTF(_raw_ptr, _data_bytes.size(), &_woff2out)) {
            LUCARIA_RUNTIME_ERROR("Impossible to decode woff2 font")
        }

        std::shared_ptr<std::vector<char>> _shared_output = std::make_shared<std::vector<char>>();
        _shared_output->assign(std::make_move_iterator(_output_str.begin()), std::make_move_iterator(_output_str.end()));
        _data_promise->set_value(std::move(_shared_output));
    });

    // create font on main thread
    return fetched<font>(_data_promise->get_future(), [font_size](const std::shared_ptr<std::vector<char>>& bytes) {
        return font(*bytes, font_size);
    });
}

}
