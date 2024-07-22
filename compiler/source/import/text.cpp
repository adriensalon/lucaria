#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include <import/text.hpp>

imported_text_data import_text(const std::filesystem::path& text_path)
{
    imported_text_data _data;
    std::ifstream _stream(text_path);
    if (!_stream) {
        std::cout << "Impossible to import shader '" << text_path.string() << "'" << std::endl;
        std::terminate();
    }
    std::stringstream _buffer;
    _buffer << _stream.rdbuf();
    _data.shader.text = _buffer.str();
    // todo extract version
    return _data;
}