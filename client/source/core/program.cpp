
#include <fstream>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <core/shader.hpp>

namespace lucaria {

shader import_shader(const std::filesystem::path& input)
{
    shader _data;
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

void compile_shader(const shader& data, const std::filesystem::path& output)
{
    std::ofstream _fstream(output);
    cereal::PortableBinaryOutputArchive _archive(_fstream);
    _archive(data.text);
}

}