#include <iostream>

#include <backends/imgui_impl_opengl3.h>
#include <woff2/decode.h>

#include <core/fetch.hpp>
#include <core/font.hpp>
#include <core/hash.hpp>

namespace detail {

// static std::vector<std::shared_ptr<ImFont>> fonts;
static std::unordered_map<std::size_t, std::pair<std::vector<std::string>, std::promise<std::shared_ptr<font_ref>>>> promises;

}

font_ref::font_ref(const font_data& data)
    : _ptrs(data)
{
}

ImFont* font_ref::get_font(const glm::uint index) const
{
    return _ptrs[index];
}

glm::uint font_ref::get_count() const
{
    return _ptrs.size();
}

std::shared_future<std::shared_ptr<font_ref>> fetch_font(const std::vector<std::filesystem::path>& font_paths, const glm::float32 font_size)
{
    const std::size_t _hash = path_vector_hash()(font_paths);
    std::pair<std::vector<std::string>, std::promise<std::shared_ptr<font_ref>>>& _promise = detail::promises[_hash];
    fetch_files(font_paths, [&_promise, font_size](std::size_t index, std::size_t total, const std::vector<char>& font_bytes) {
        const std::uint8_t* _raw_ptr = reinterpret_cast<const uint8_t*>(font_bytes.data());
        std::string _output_str(std::min(woff2::ComputeWOFF2FinalSize(_raw_ptr, font_bytes.size()), woff2::kDefaultMaxSize), 0);
        woff2::WOFF2StringOut _woff2out(&_output_str);
        if (!woff2::ConvertWOFF2ToTTF(_raw_ptr, font_bytes.size(), &_woff2out)) {
#if LUCARIA_DEBUG
            std::cout << "Impossible to decode woff2 font." << std::endl;
            std::terminate();
#endif
        }
        // if (detail::fonts.empty()) {
        //     detail::fonts.emplace_back(std::shared_ptr<ImFont>(ImGui::GetIO().Fonts->AddFontDefault()));
        // }

        _promise.first.push_back(_output_str);

        // detail::fonts.emplace_back(_font_ptr);
        // ImGui::GetIO().Fonts->AddFontDefault();
        // ImGui::GetIO().Fonts->

        if (_promise.first.size() == total) {
            std::vector<ImFont*> _fonts;
            for (std::string& _str : _promise.first) {
                _fonts.emplace_back(ImGui::GetIO().Fonts->AddFontFromMemoryTTF(_str.data(), _str.size(), font_size));
            }
            ImGui::GetIO().Fonts->Build();
            ImGui_ImplOpenGL3_DestroyFontsTexture(); // un peu sale mdr
            ImGui_ImplOpenGL3_CreateFontsTexture();
            _promise.second.set_value(std::make_shared<font_ref>(_fonts));
        }
    });

    return _promise.second.get_future();
}