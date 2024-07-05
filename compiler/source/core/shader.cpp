#include <iostream>
#include <fstream>
#include <sstream>

#include <core/import.hpp>

shader_data import_shader(const std::filesystem::path& input)
{
    shader_data _data;
    std::ifstream _stream(input);
    if (!_stream) {
        std::cout << "Impossible to import shader '" << input << "'" << std::endl;
        std::terminate();
    }
    std::stringstream _buffer;
    _buffer << _stream.rdbuf();
    _data.text = _buffer.str();
    return _data;
}
