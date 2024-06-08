#include <fstream>
#include <filesystem>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <stb_image.h>

#include <data/texture.hpp>

/// @brief Imports a texture as data
/// @param input the texture path to load, containing 3 (RGB) or 4 (RGBA) channels
/// @return the texture data with pixels stored as unsigned char (0-255)
texture_data import_texture(const std::filesystem::path& input)
{
    texture_data _data;
    int _width, _height, _channels;
    unsigned char* _pixels = stbi_load(input.generic_string().c_str(), &_width, &_height, &_channels, NULL);
    if (_pixels == nullptr) {
        std::cerr << "Impossible to import texture '" << input.generic_string() << "'" << std::endl;
        std::terminate();
    }
    _data.channels = static_cast<unsigned int>(_channels);
    _data.width = static_cast<unsigned int>(_width);
    _data.height = static_cast<unsigned int>(_height);
    _data.pixels = std::vector<unsigned char>(_pixels, _pixels + (_data.width * _data.height * _data.channels));
    stbi_image_free(_pixels);
    return _data;
}

/// @brief Compiles a texture to a binary file
/// @param data the texture data to compile as binary
/// @param output the binary path to save the texture to
void compile_texture(const texture_data& data, const std::filesystem::path& output)
{
    std::ofstream _fstream(output);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
    _archive(data.channels);
    _archive(data.width);
    _archive(data.height);
    _archive(data.pixels);
}