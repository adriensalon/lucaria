#include <filesystem>
#include <iostream>

#include <stb_image.h>

#include <data/texture.hpp>

image_data import_texture(const std::filesystem::path& image_path)
{
    image_data _data;
    int _width, _height, _channels;
    unsigned char* _pixels = stbi_load(image_path.string().c_str(), &_width, &_height, &_channels, NULL);
    if (_pixels == nullptr) {
        std::cout << "Impossible to import texture '" << image_path << "'" << std::endl;
        std::terminate();
    }
    _data.channels = static_cast<unsigned int>(_channels);
    _data.width = static_cast<unsigned int>(_width);
    _data.height = static_cast<unsigned int>(_height);
    _data.pixels = std::vector<unsigned char>(_pixels, _pixels + (_data.width * _data.height * _data.channels));
    stbi_image_free(_pixels);
    std::cout << "   Exporting texture data binary..." << std::endl;
    return _data;
}
