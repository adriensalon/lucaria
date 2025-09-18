#include <iostream>

#include <backends/imgui_impl_opengl3.h>
#include <woff2/decode.h>

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/font.hpp>
#include <lucaria/core/hash.hpp>

namespace detail {

static std::unordered_map<std::size_t, std::pair<std::vector<std::string>, std::promise<std::shared_ptr<font_ref>>>> promises;

}

font_ref::font_ref(const font_data& data, const glm::float32 font_size)
{
    for (const std::string& _ttf_str : data) {
        void* _void_data_ptr = const_cast<void*>(static_cast<const void*>(_ttf_str.data()));
        _ptrs.emplace_back(ImGui::GetIO().Fonts->AddFontFromMemoryTTF(_void_data_ptr, _ttf_str.size(), static_cast<int>(font_size)));
    }
    if (!ImGui::GetIO().Fonts->Build()) {
#if LUCARIA_DEBUG
        std::cout << "Impossible to build ImGui font atlas." << std::endl;
        std::terminate();
#endif
    }
    ImGui_ImplOpenGL3_DestroyFontsTexture(); // un peu sale mdr
    ImGui_ImplOpenGL3_CreateFontsTexture();
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
    std::pair<std::vector<std::string>, std::promise<std::shared_ptr<font_ref>>>& _promise_pair = detail::promises[_hash];
    fetch_files(font_paths, [&_promise_pair, font_size](std::size_t index, std::size_t total, const std::vector<char>& font_bytes) {
        const std::uint8_t* _raw_ptr = reinterpret_cast<const uint8_t*>(font_bytes.data());
        std::string& _output_str = _promise_pair.first.emplace_back(std::min(woff2::ComputeWOFF2FinalSize(_raw_ptr, font_bytes.size()), woff2::kDefaultMaxSize), 0);
        woff2::WOFF2StringOut _woff2out(&_output_str);
        if (!woff2::ConvertWOFF2ToTTF(_raw_ptr, font_bytes.size(), &_woff2out)) {
#if LUCARIA_DEBUG
            std::cout << "Impossible to decode woff2 font." << std::endl;
            std::terminate();
#endif
        }
        if (_promise_pair.first.size() == total) {
            _promise_pair.second.set_value(std::make_shared<font_ref>(_promise_pair.first, font_size));
        }
    });
    return _promise_pair.second.get_future();
}

void clear_font_fetches()
{
    detail::promises.clear();
}