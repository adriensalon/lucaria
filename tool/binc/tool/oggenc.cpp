#include <cstdio>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include <audio.h>
#include <encode.h>
}

#include "../export/binary.hpp"
#include "oggenc.hpp"

namespace {

unsigned int next_ogg_serial()
{
    static unsigned int _serial = static_cast<unsigned int>(std::time(nullptr));
    return ++_serial;
}

void close_audio_format(input_format* format, oe_enc_opt& options)
{
    if (format && format->close_func) {
        format->close_func(options.readdata);
    }
}

std::vector<lucaria::uint8> read_binary_file(const std::filesystem::path& path)
{
    std::ifstream _file(path, std::ios::binary);
    if (!_file) {
        std::cout << "Could not open encoded audio file " << path << std::endl;
        std::terminate();
    }

    _file.seekg(0, std::ios::end);
    const std::streampos _end = _file.tellg();
    _file.seekg(0, std::ios::beg);

    if (_end <= std::streampos(0)) {
        std::cout << "Encoded audio file is empty " << path << std::endl;
        std::terminate();
    }

    std::vector<lucaria::uint8> _data(static_cast<std::size_t>(_end));
    if (!_file.read(reinterpret_cast<char*>(_data.data()), static_cast<std::streamsize>(_data.size()))) {
        std::cout << "Could not read encoded audio file " << path << std::endl;
        std::terminate();
    }
    return _data;
}

}

void execute_oggenc(const std::filesystem::path& input_path, const std::filesystem::path& output_path)
{
    const std::filesystem::path _encoded_path = output_path.string() + ".ogg.tmp";
    const std::string _input_name = input_path.string();
    const std::string _encoded_name = _encoded_path.string();

    FILE* _input_file = std::fopen(_input_name.c_str(), "rb");
    if (!_input_file) {
        std::cout << "Could not open audio file " << input_path << std::endl;
        std::terminate();
    }

    vorbis_comment _comments;
    vorbis_comment_init(&_comments);

    oe_enc_opt _options = {};
    _options.serialno = next_ogg_serial();
    _options.skeleton_serialno = _options.serialno + 1;
    _options.kate_serialno = _options.skeleton_serialno + 1;
    _options.progress_update = update_statistics_null;
    _options.start_encode = start_encode_null;
    _options.end_encode = final_statistics_null;
    _options.error = encode_error;
    _options.comments = &_comments;
    _options.copy_comments = 1;
    _options.with_skeleton = 0;
    _options.ignorelength = 0;

    input_format* _format = open_audio_file(_input_file, &_options);
    if (!_format) {
        std::cout << "Unsupported audio file " << input_path << std::endl;
        vorbis_comment_clear(&_comments);
        std::fclose(_input_file);
        std::terminate();
    }

    if (_options.rate <= 0) {
        std::cout << "Audio file has invalid sample rate " << input_path << std::endl;
        close_audio_format(_format, _options);
        vorbis_comment_clear(&_comments);
        std::fclose(_input_file);
        std::terminate();
    }

    FILE* _output_file = std::fopen(_encoded_name.c_str(), "wb");
    if (!_output_file) {
        std::cout << "Could not open compressed audio output " << _encoded_path << std::endl;
        close_audio_format(_format, _options);
        vorbis_comment_clear(&_comments);
        std::fclose(_input_file);
        std::terminate();
    }

    bool _downmixed = false;
    if (_options.channels == 2) {
        setup_downmix(&_options);
        _downmixed = true;
    }

    _options.out = _output_file;
    _options.filename = const_cast<char*>(_encoded_name.c_str());
    _options.infilename = const_cast<char*>(_input_name.c_str());
    _options.managed = 0;
    _options.bitrate = -1;
    _options.min_bitrate = -1;
    _options.max_bitrate = -1;
    _options.quality = 0.3f;
    _options.quality_set = -1;
    _options.advopt = nullptr;
    _options.advopt_count = 0;
    _options.lyrics = nullptr;
    _options.lyrics_language = nullptr;

    const int _result = oe_encode(&_options);

    if (_downmixed) {
        clear_downmix(&_options);
    }
    close_audio_format(_format, _options);
    vorbis_comment_clear(&_comments);
    std::fclose(_output_file);
    std::fclose(_input_file);

    if (_result != 0) {
        std::cout << "Ogg Vorbis encoding failed for " << input_path << std::endl;
        std::terminate();
    }

    lucaria::data_audio _audio_data;
    _audio_data.profile = lucaria::data_audio_profile::ogg_vorbis;
    _audio_data.sample_rate = static_cast<lucaria::uint32>(_options.rate);
    _audio_data.channels = static_cast<lucaria::uint32>(_downmixed ? 1 : _options.channels);
    _audio_data.count = _options.total_samples_per_channel > 0 ? static_cast<lucaria::uint32>(_options.total_samples_per_channel) : 0;
    _audio_data.samples = read_binary_file(_encoded_path);
    std::filesystem::remove(_encoded_path);

    export_binary(_audio_data, output_path);
    std::cout << "   Exporting ogg compressed audio " << output_path.filename() << std::endl;
}
