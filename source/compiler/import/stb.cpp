#include <iostream>

#include <stb_image.h>

#include "stb.hpp"

image_data import_stb(const std::filesystem::path& stb_path)
{
    image_data _data;
    int _width, _height, _channels;
    unsigned char* _pixels = stbi_load(stb_path.string().c_str(), &_width, &_height, &_channels, NULL);
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