#include <iostream>

#include <backends/imgui_impl_opengl3.h>
#include <woff2/decode.h>

#include <core/fetch.hpp>
#include <core/font.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<font_ref>>> promises;

}

std::shared_future<std::shared_ptr<font_ref>> fetch_font(const std::filesystem::path& font_path)
{
    std::promise<std::shared_ptr<font_ref>>& _promise = detail::promises[font_path.string()];
    fetch_file(font_path, [&_promise](const std::vector<char>& font_bytes) {
        const std::uint8_t* _raw_ptr = reinterpret_cast<const uint8_t*>(font_bytes.data());
        std::string _output_str(std::min(woff2::ComputeWOFF2FinalSize(_raw_ptr, font_bytes.size()), woff2::kDefaultMaxSize), 0);
        woff2::WOFF2StringOut _woff2out(&_output_str);
        if (!woff2::ConvertWOFF2ToTTF(_raw_ptr, font_bytes.size(), &_woff2out)) {
#if LUCARIA_DEBUG
            std::cout << "Impossible to decode woff2 font." << std::endl;
            std::terminate();
#endif
        }
        _promise.set_value(std::shared_ptr<ImFont>(ImGui::GetIO().Fonts->AddFontFromMemoryTTF(_output_str.data(), _output_str.size(), 16.f)));
//         ImGui::GetIO().Fonts->AddFontDefault();
//         if (!ImGui::GetIO().Fonts->Build()) {
// #if LUCARIA_DEBUG
//             std::cout << "Impossible build ImGui font." << std::endl;
//             std::terminate();
// #endif
//         } else {
//             ImGui_ImplOpenGL3_DestroyFontsTexture(); // un peu sale mdr
//             ImGui_ImplOpenGL3_CreateFontsTexture();
//         }
    });

    return _promise.get_future();
}