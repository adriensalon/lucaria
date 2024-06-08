#include <fstream>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <stb_image.h>
#include <core/texture.hpp>

namespace lucaria {

texture import_texture(const std::filesystem::path& input)
{
    texture _data;
    int _width, _height, _channels;
    unsigned char* _pixels = stbi_load(input.generic_string().c_str(), &_width, &_height, &_channels, NULL);
    if (_pixels == nullptr) {
        std::cerr << "Impossible to import texture '" << input.generic_string() << "'" << std::endl;
        std::terminate();
    }
    _data.channels = _channels;
    _data.width = _width;
    _data.height = _height;
    _data.pixels = std::vector<unsigned char>(_pixels, _pixels + (_data.width * _data.height * _data.channels));
    stbi_image_free(_pixels);
    return _data;
}

void compile_texture(const texture& data, const std::filesystem::path& output)
{
    std::ofstream _fstream(output);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
    _archive(data.channels);
    _archive(data.width);
    _archive(data.height);
    _archive(data.pixels);
}

}