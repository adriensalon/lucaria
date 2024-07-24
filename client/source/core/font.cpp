#include <iostream>

#include <woff2/decode.h>

#include <core/font.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<font_ref>>> promises;

}

std::shared_future<std::shared_ptr<font_ref>> fetch_font(const std::filesystem::path& font_path)
{
    std::promise<std::shared_ptr<font_ref>>& _promise = detail::promises[font_path.string()];


    return _promise.get_future();
}