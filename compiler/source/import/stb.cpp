#include <iostream>

#include <stb_image.h>

#include <import/stb.hpp>

imported_stb_data import_stb(const std::filesystem::path& stb_path)
{
    imported_stb_data _data;
    _data.is_hdr = static_cast<bool>(stbi_is_hdr(stb_path.string().c_str()));
    int _width, _height, _channels;
    unsigned char* _pixels = stbi_load(stb_path.string().c_str(), &_width, &_height, &_channels, NULL);
    if (_pixels == nullptr) {
        std::cout << "Impossible to import texture '" << stb_path << "'" << std::endl;
        std::terminate();
    }
    _data.image.channels = static_cast<unsigned int>(_channels);
    _data.image.width = static_cast<unsigned int>(_width);
    _data.image.height = static_cast<unsigned int>(_height);
    _data.image.pixels = std::vector<unsigned char>(_pixels, _pixels + (_data.image.width * _data.image.height * _data.image.channels));
    stbi_image_free(_pixels);
    return _data;
}