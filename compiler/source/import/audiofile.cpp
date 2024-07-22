
#include <iostream>

#include <AudioFile.h>

#include <import/audiofile.hpp>

imported_audiofile_data import_audiofile(const std::filesystem::path& audiofile_path)
{
    imported_audiofile_data _data;
    AudioFile<float> _audiofile;
    if (!_audiofile.load(audiofile_path.string())) {
        std::cout << "Impossible to load audio file " << audiofile_path << "." << std::endl;
        std::terminate();
    }
    _data.sample_rate = _audiofile.getSampleRate();
    _data.bit_depth = _audiofile.getBitDepth();
    _data.song_audio.channels = _audiofile.getNumChannels();
    _data.song_audio.length = _audiofile.getNumSamplesPerChannel();
    _data.song_audio.samples.resize(_data.song_audio.channels * _data.song_audio.length);
    for (glm::uint _channel = 0; _channel < _data.song_audio.channels; ++_channel) {
        for (glm::uint _sample = 0; _sample < _data.song_audio.length; ++_sample) {
            _data.song_audio.samples[_channel * _data.song_audio.channels + _sample] = _audiofile.samples[_channel][_sample];
        }
    }
    return _data;
}
