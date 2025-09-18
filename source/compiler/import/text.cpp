#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "text.hpp"

shader_data import_text(const std::filesystem::path& text_path)
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
    return _data;
}