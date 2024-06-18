#include <fstream>
#include <filesystem>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

#include <data/shader.hpp>

/// @brief Imports a shader as data
/// @param input the shader path to load, containing text
/// @return the shader data as a string
shader_data import_shader(const std::filesystem::path& input)
{
    shader_data _data;
    std::ifstream _stream(input);
    if (!_stream) {
        std::cerr << "Impossible to import shader '" << input.generic_string() << "'" << std::endl;
        std::terminate();
    }
    std::stringstream _buffer;
    _buffer << _stream.rdbuf();
    _data.text = _buffer.str();
    return _data;
}
