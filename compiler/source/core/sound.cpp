#include <AudioFile.h>

#include <compile/sound.hpp>
#include <data/sound.hpp>
#include <glue/console.hpp>
#include <glue/exceptions.hpp>
#include <glue/file.hpp>
#include <glue/serialization.hpp>

namespace webgame {
using namespace common::audio;

namespace compiler {
    using namespace common;

    void compile_sound(const json::value& json_root, const std::filesystem::path& output_path)
    {
        // options
        std::filesystem::path _audio_path;
        _audio_path = std::filesystem::path(json::get_string(json_root, "path", [&]() {
            console::log("error : no field 'path' provided for audio", console::text_color::red);
            exceptions::throw_error();
        }));
        file::assert_file_exists(_audio_path);

        // import
        sound_data _data;
        AudioFile<float> _audio_file;
        if (!_audio_file.load(_audio_path.generic_string())) {
            console::log("error : while loading audio file '" + output_path.generic_string() + "'", console::text_color::red);
            exceptions::throw_error();
        }
        _data.bits = _audio_file.getBitDepth();
        _data.rate = _audio_file.getSampleRate();
        _data.channels = _audio_file.getNumChannels();
        _data.count = _audio_file.getNumSamplesPerChannel();
        for (unsigned int _l = 0; _l < _data.count; _l++)
            for (unsigned int _k = 0; _k < _data.channels; _k++)
                _data.samples.emplace_back(_audio_file.samples[_k][_l]);

        // std::cout << "Bit Depth: " << _data.bits << "mais osef" << std::endl;
        // std::cout << "Sample Rate: " << _data.rate << std::endl;
        // std::cout << "Num Channels: " << _data.channels << std::endl;
        // std::cout << "Samples Count: " << _data.count << std::endl;

        // save
        save(output_path, _data);
    }

}
}