#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#include <core/fetch.hpp>
#include <core/sound.hpp>
#include <core/window.hpp>

namespace detail {

struct VorbisStream {
    std::istringstream& stream;
    std::vector<char> buffer;

    VorbisStream(std::istringstream& data_stream)
        : stream(data_stream)
    {
        stream.seekg(0, std::ios::end);
        size_t _size = stream.tellg();
        buffer.resize(_size);
        stream.seekg(0, std::ios::beg);
        stream.read(buffer.data(), _size);
        stream.str(std::string(buffer.begin(), buffer.end()));
    }
};

size_t read_func(void* ptr, size_t size, size_t nmemb, void* data_source)
{
    VorbisStream* _stream = static_cast<VorbisStream*>(data_source);
    _stream->stream.read(static_cast<char*>(ptr), size * nmemb);
    return _stream->stream.gcount();
}

int seek_func(void* data_source, ogg_int64_t offset, int whence)
{
    VorbisStream* _stream = static_cast<VorbisStream*>(data_source);
    std::istringstream::pos_type _pos;
    switch (whence) {
    case SEEK_SET:
        _pos = offset;
        break;
    case SEEK_CUR:
        _pos = _stream->stream.tellg() + std::istringstream::pos_type(offset);
        break;
    case SEEK_END:
        _pos = _stream->buffer.size() - offset;
        break;
    default:
        return -1;
    }
    _stream->stream.clear();
    _stream->stream.seekg(_pos);
    return _stream->stream.good() ? 0 : -1;
}

long tell_func(void* data_source)
{
    VorbisStream* _stream = static_cast<VorbisStream*>(data_source);
    return static_cast<long>(_stream->stream.tellg());
}

int close_func(void* data_source)
{
    return 0;
}

static std::unordered_map<std::string, std::promise<std::shared_ptr<sound_ref>>> promises;

}

sound_ref::sound_ref(sound_ref&& other)
{
    *this = std::move(other);
}

sound_ref& sound_ref::operator=(sound_ref&& other)
{
    _buffer_id = other._buffer_id;
    _is_instanced = true;
    other._is_instanced = false;
    return *this;
}

sound_ref::~sound_ref()
{
    if (_is_instanced) {
        alDeleteBuffers(1, &_buffer_id);
    }
}

sound_ref::sound_ref(const audio_data& data)
{
    _buffer_id = 0;
    alGenBuffers(1, &_buffer_id);
#if LUCARIA_DEBUG
    if (_buffer_id == 0) {
        std::cout << "Failed to generate OpenAL buffer." << std::endl;
        std::terminate();
    }
#endif
    alBufferData(_buffer_id, alGetEnumValue("AL_FORMAT_MONO_FLOAT32"), data.samples.data(), static_cast<ALsizei>(data.samples.size() * sizeof(glm::float32)), data.sample_rate);
#if LUCARIA_DEBUG
    audio_assert();
    std::cout << "Created sound buffer of size " << data.samples.size() << " with id " << _buffer_id << std::endl;
#endif
    _is_instanced = true;
}

glm::uint sound_ref::get_id() const
{
    return _buffer_id;
}

audio_data load_audio_data(std::istringstream& audio_stream)
{
    audio_data _data;
    detail::VorbisStream _stream(audio_stream);
    OggVorbis_File _vorbis;
    ov_callbacks _callbacks;
    _callbacks.read_func = detail::read_func;
    _callbacks.seek_func = detail::seek_func;
    _callbacks.tell_func = detail::tell_func;
    _callbacks.close_func = detail::close_func;
    if (ov_open_callbacks(&_stream, &_vorbis, nullptr, 0, _callbacks) != 0) {
        std::cout << "Invalid Ogg file." << std::endl;
        std::terminate();
    }
    vorbis_info* _info = ov_info(&_vorbis, -1);
    if (_info->channels != 1) {
        std::cout << "Only mono files are supported." << std::endl;
        ov_clear(&_vorbis);
        std::terminate();
    }
    _data.sample_rate = _info->rate;
    float** _pcm_channels;
    int _bitstream;
    long _samples;
    while ((_samples = ov_read_float(&_vorbis, &_pcm_channels, 512, &_bitstream)) > 0) {
        for (long i = 0; i < _samples; ++i) {
            _data.samples.push_back(_pcm_channels[0][i]);
        }
    }
    if (_samples < 0) {
        std::cout << "Error reading Ogg file." << std::endl;
        std::terminate();
    }
    ov_clear(&_vorbis);
    return _data;
}

std::shared_future<std::shared_ptr<sound_ref>> fetch_sound(const std::filesystem::path& sound_path)
{
    std::promise<std::shared_ptr<sound_ref>>& _promise = detail::promises[sound_path.string()];
    on_audio_locked([&_promise, sound_path] () {
        fetch_file(sound_path, [&_promise](std::istringstream& stream) {
            _promise.set_value(std::move(std::make_shared<sound_ref>(load_audio_data(stream))));
        });
    });
    return _promise.get_future();
}
