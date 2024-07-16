#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <data/shader.hpp>

shader_data import_shader(const std::filesystem::path& text_path)
{
    shader_data _data;
    std::ifstream _stream(text_path);
    if (!_stream) {
        std::cout << "Impossible to import shader '" << text_path.string() << "'" << std::endl;
        std::terminate();
    }
    std::stringstream _buffer;
    _buffer << _stream.rdbuf();
    _data.text = _buffer.str();
    std::cout << "   Exporting shader data binary..." << std::endl;
    return _data;
}
