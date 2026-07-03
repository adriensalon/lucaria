#include <algorithm>
#include <cstddef>
#include <iostream>

#include <stb_image.h>
#include <stb_image_resize2.h>
#include <stb_image_write.h>

#include <binc/import_stb.hpp>

namespace {

lucaria::data_image resize_image(const lucaria::data_image& image, const lucaria::uint32 max_size)
{
    const lucaria::uint32 _largest_dimension = std::max(image.width, image.height);
    if (max_size == 0 || _largest_dimension <= max_size) {
        return image;
    }

    lucaria::data_image _resized = image;
    _resized.width = std::max<lucaria::uint32>(1, (image.width * max_size + _largest_dimension / 2) / _largest_dimension);
    _resized.height = std::max<lucaria::uint32>(1, (image.height * max_size + _largest_dimension / 2) / _largest_dimension);
    _resized.pixels.resize(static_cast<std::size_t>(_resized.width) * _resized.height * _resized.channels);

    const bool _success = stbir_resize_uint8_linear(
        image.pixels.data(),
        static_cast<int>(image.width),
        static_cast<int>(image.height),
        0,
        _resized.pixels.data(),
        static_cast<int>(_resized.width),
        static_cast<int>(_resized.height),
        0,
        static_cast<stbir_pixel_layout>(image.channels)) != nullptr;
    if (!_success) {
        std::cout << "Impossible to resize texture" << std::endl;
        std::terminate();
    }
    return _resized;
}

}

lucaria::data_image import_stb(const std::filesystem::path& stb_path)
{
    lucaria::data_image _data;
    int _width, _height, _channels;
    unsigned char* _pixels = stbi_load(stb_path.string().c_str(), &_width, &_height, &_channels, 0);
    if (_pixels == nullptr) {
        std::cout << "Impossible to import texture '" << stb_path << "'" << std::endl;
        std::terminate();
    }
    _data.channels = static_cast<unsigned int>(_channels);
    _data.width = static_cast<unsigned int>(_width);
    _data.height = static_cast<unsigned int>(_height);
    _data.pixels = std::vector<unsigned char>(_pixels, _pixels + (_data.width * _data.height * _data.channels));
    // std::cout << "tex W " << _width << " H " << _height << " C " << _channels << std::endl;
    stbi_image_free(_pixels);
    return _data;
}

lucaria::data_image import_stb(const std::filesystem::path& stb_path, const lucaria::uint32 max_size)
{
    return resize_image(import_stb(stb_path), max_size);
}

void write_stb_png(const lucaria::data_image& image, const std::filesystem::path& stb_path)
{
    const int _stride = static_cast<int>(image.width * image.channels);
    if (stbi_write_png(stb_path.string().c_str(), static_cast<int>(image.width), static_cast<int>(image.height), static_cast<int>(image.channels), image.pixels.data(), _stride) == 0) {
        std::cout << "Impossible to write texture '" << stb_path << "'" << std::endl;
        std::terminate();
    }
}
