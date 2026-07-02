#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

#include <woff2/encode.h>

#include <binc/compile_woff2enc.hpp>

namespace {

std::vector<uint8_t> read_binary_file(const std::filesystem::path& path)
{
    std::ifstream _file(path, std::ios::binary);
    if (!_file) {
        std::cout << "Could not open font file " << path << std::endl;
        std::terminate();
    }

    _file.seekg(0, std::ios::end);
    const std::streampos _end = _file.tellg();
    _file.seekg(0, std::ios::beg);

    if (_end <= std::streampos(0)) {
        std::cout << "Font file is empty " << path << std::endl;
        std::terminate();
    }

    const size_t _size = static_cast<size_t>(_end);
    std::vector<uint8_t> _data(static_cast<size_t>(_size));
    if (!_file.read(reinterpret_cast<char*>(_data.data()), static_cast<std::streamsize>(_data.size()))) {
        std::cout << "Could not read font file " << path << std::endl;
        std::terminate();
    }
    return _data;
}

void write_binary_file(const std::filesystem::path& path, const std::vector<uint8_t>& data)
{
    std::ofstream _file(path, std::ios::binary);
    if (!_file) {
        std::cout << "Could not open compressed font output " << path << std::endl;
        std::terminate();
    }

    _file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!_file) {
        std::cout << "Could not write compressed font output " << path << std::endl;
        std::terminate();
    }
}

}

void execute_woff2compress(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
{
    const std::vector<uint8_t> _input = read_binary_file(input_path);
    size_t _output_size = woff2::MaxWOFF2CompressedSize(_input.data(), _input.size());
    std::vector<uint8_t> _output(_output_size);

    woff2::WOFF2Params _params;
    if (!woff2::ConvertTTFToWOFF2(_input.data(), _input.size(), _output.data(), &_output_size, _params)) {
        std::cout << "WOFF2 compression failed for " << input_path << std::endl;
        std::terminate();
    }

    _output.resize(_output_size);
    write_binary_file(output_path, _output);
    std::cout << "   Exporting woff2 compressed font " << output_path.filename() << std::endl;
}
