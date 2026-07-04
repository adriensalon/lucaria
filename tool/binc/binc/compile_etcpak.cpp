#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <binc/compile_bin.hpp>
#include <binc/compile_etcpak.hpp>
#include <lucaria/bin/data_image.hpp>

int lucaria_etcpak_main(int argc, char** argv);

extern "C" {
extern char* optarg;
extern int opterr;
extern int optind;
extern int optopt;
}

namespace detail {

std::vector<lucaria::uint8> read_bytes(const std::filesystem::path& path)
{
    std::ifstream _stream(path, std::ios::binary | std::ios::ate);
    if (!_stream) {
        std::cout << "Could not open etcpak output " << path << std::endl;
        std::terminate();
    }
    const std::streamsize _size = _stream.tellg();
    _stream.seekg(0, std::ios::beg);
    std::vector<lucaria::uint8> _bytes(static_cast<std::size_t>(_size));
    if (!_stream.read(reinterpret_cast<char*>(_bytes.data()), _size)) {
        std::cout << "Could not read etcpak output " << path << std::endl;
        std::terminate();
    }
    return _bytes;
}

void require_words(const std::vector<lucaria::uint8>& bytes, const std::size_t count)
{
    if (bytes.size() < count * sizeof(std::uint32_t)) {
        std::cout << "Invalid etcpak output, file is too small" << std::endl;
        std::terminate();
    }
}

std::uint32_t read_u32(const std::vector<lucaria::uint8>& bytes, const std::size_t index)
{
    require_words(bytes, index + 1);
    std::uint32_t _value = 0;
    std::memcpy(&_value, bytes.data() + index * sizeof(std::uint32_t), sizeof(_value));
    return _value;
}

lucaria::data_image import_pvr(const std::vector<lucaria::uint8>& bytes)
{
    require_words(bytes, 13);
    lucaria::data_image _data;
    switch (read_u32(bytes, 2)) {
    case 7:
        _data.channels = 3;
        _data.profile = lucaria::data_image_profile::s3tc_rgb4;
        break;
    case 11:
        _data.channels = 4;
        _data.profile = lucaria::data_image_profile::s3tc_rgba8;
        break;
    case 22:
        _data.channels = 3;
        _data.profile = lucaria::data_image_profile::etc2_rgb4;
        break;
    case 23:
        _data.channels = 4;
        _data.profile = lucaria::data_image_profile::etc2_rgba8;
        break;
    default:
        std::cout << "Invalid PVR image data" << std::endl;
        std::terminate();
    }
    const std::size_t _offset = 52 + read_u32(bytes, 12);
    if (_offset > bytes.size()) {
        std::cout << "Invalid PVR image data offset" << std::endl;
        std::terminate();
    }
    _data.height = read_u32(bytes, 6);
    _data.width = read_u32(bytes, 7);
    _data.pixels = std::vector<lucaria::uint8>(bytes.data() + _offset, bytes.data() + bytes.size());
    return _data;
}

lucaria::data_image import_ktx(const std::vector<lucaria::uint8>& bytes)
{
    require_words(bytes, 17);
    lucaria::data_image _data;
    switch (read_u32(bytes, 7)) {
    case 0x9274:
        _data.channels = 3;
        _data.profile = lucaria::data_image_profile::etc2_rgb4;
        break;
    case 0x9278:
        _data.channels = 4;
        _data.profile = lucaria::data_image_profile::etc2_rgba8;
        break;
    default:
        std::cout << "Invalid KTX image data" << std::endl;
        std::terminate();
    }
    const std::uint32_t _bytes_of_key_value = read_u32(bytes, 15);
    std::size_t _offset = sizeof(std::uint32_t) * 16 + _bytes_of_key_value;
    if (_offset + sizeof(std::uint32_t) > bytes.size()) {
        std::cout << "Invalid KTX image data offset" << std::endl;
        std::terminate();
    }
    std::uint32_t _image_size = 0;
    std::memcpy(&_image_size, bytes.data() + _offset, sizeof(_image_size));
    _offset += sizeof(std::uint32_t);
    if (_offset + _image_size > bytes.size()) {
        std::cout << "Invalid KTX image data size" << std::endl;
        std::terminate();
    }
    _data.width = read_u32(bytes, 9);
    _data.height = read_u32(bytes, 10);
    _data.pixels = std::vector<lucaria::uint8>(bytes.data() + _offset, bytes.data() + _offset + _image_size);
    return _data;
}

lucaria::data_image import_etcpak_output(const std::filesystem::path& path)
{
    const std::vector<lucaria::uint8> _bytes = read_bytes(path);
    const std::uint32_t _identifier = read_u32(_bytes, 0);
    if (_identifier == 0x03525650) {
        return import_pvr(_bytes);
    }
    if (_identifier == 0x58544BAB) {
        return import_ktx(_bytes);
    }
    std::cout << "Invalid etcpak output image data" << std::endl;
    std::terminate();
}

void convert_dxt1_to_psp(std::vector<lucaria::uint8>& pixels)
{
    for (std::size_t _offset = 0; _offset + 8 <= pixels.size(); _offset += 8) {
        lucaria::uint8 _block[8] = {};
        std::memcpy(_block, pixels.data() + _offset, sizeof(_block));
        std::memcpy(pixels.data() + _offset + 0, _block + 4, 4);
        std::memcpy(pixels.data() + _offset + 4, _block + 0, 4);
    }
}

void convert_dxt5_to_psp(std::vector<lucaria::uint8>& pixels)
{
    for (std::size_t _offset = 0; _offset + 16 <= pixels.size(); _offset += 16) {
        lucaria::uint8 _block[16] = {};
        std::memcpy(_block, pixels.data() + _offset, sizeof(_block));
        std::memcpy(pixels.data() + _offset + 0, _block + 12, 4);
        std::memcpy(pixels.data() + _offset + 4, _block + 8, 4);
        std::memcpy(pixels.data() + _offset + 8, _block + 2, 6);
        std::memcpy(pixels.data() + _offset + 14, _block + 0, 2);
    }
}

void convert_s3tc_to_psp(lucaria::data_image& data)
{
    switch (data.profile) {
    case lucaria::data_image_profile::s3tc_rgb4:
        convert_dxt1_to_psp(data.pixels);
        break;
    case lucaria::data_image_profile::s3tc_rgba8:
        convert_dxt5_to_psp(data.pixels);
        break;
    default:
        break;
    }
}

}

void execute_etcpak(const etcpak_mode mode, const std::filesystem::path& input_path, const std::filesystem::path& output_path)
{
    const std::filesystem::path _temporary_output_path = output_path.string() + ".etcpak.tmp";
    std::vector<std::string> _arguments_storage = {
        "lucaria_etcpak",
        "--disable-heuristics" // better quality
    };
    if (mode == etcpak_mode::s3tc || mode == etcpak_mode::s3tc_psp) { // no --etc2 (its ETC2 by default)
        _arguments_storage.emplace_back("--dxtc");
    }
    _arguments_storage.emplace_back(input_path.string());
    _arguments_storage.emplace_back(_temporary_output_path.string());

    std::vector<char*> _arguments;
    _arguments.reserve(_arguments_storage.size());
    for (std::string& _argument : _arguments_storage) {
        _arguments.emplace_back(_argument.data());
    }

    optarg = nullptr;
    opterr = 0;
    optind = 1;
    optopt = 0;

    const int _result = lucaria_etcpak_main(static_cast<int>(_arguments.size()), _arguments.data());
    if (_result != 0) {
        std::cout << "Etcpak conversion failed with exit code " << _result << "." << std::endl;
        std::terminate();
    }
    lucaria::data_image _data = detail::import_etcpak_output(_temporary_output_path);
    if (mode == etcpak_mode::s3tc_psp) {
        detail::convert_s3tc_to_psp(_data);
    }
    std::filesystem::remove(_temporary_output_path);
    export_binary(_data, output_path);
}
