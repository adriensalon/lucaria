#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <vorbis/vorbisfile.h>

#include <lucaria/core/audio.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/sound.hpp>
#include <lucaria/core/window.hpp>

namespace detail {
    
class VorbisStream {
public:
    VorbisStream(const std::vector<char>& data)
        : data_(data), position_(0) {}

    ~VorbisStream() = default;

    size_t Read(void* ptr, size_t size, size_t nmemb) {
        size_t remaining = data_.size() - position_;
        size_t to_read = std::min(size * nmemb, remaining);
        std::memcpy(ptr, data_.data() + position_, to_read);
        position_ += to_read;
        return to_read / size;  // Return the number of items read
    }

    int Seek(ogg_int64_t offset, int whence) {
        ogg_int64_t new_position = 0;
        switch (whence) {
            case SEEK_SET:
                new_position = offset;
                break;
            case SEEK_CUR:
                new_position = position_ + offset;
                break;
            case SEEK_END:
                new_position = data_.size() + offset;
                break;
            default:
                return -1;
        }
        if (new_position < 0 || static_cast<size_t>(new_position) > data_.size()) {
            return -1;
        }
        position_ = new_position;
        return 0;
    }

    long Tell() const {
        return static_cast<long>(position_);
    }

private:
    const std::vector<char>& data_;
    size_t position_;
};

// Read function for Vorbis decoder
size_t read_func(void* ptr, size_t size, size_t nmemb, void* data_source) {
    VorbisStream* stream = static_cast<VorbisStream*>(data_source);
    return stream->Read(ptr, size, nmemb);
}

// Seek function for Vorbis decoder
int seek_func(void* data_source, ogg_int64_t offset, int whence) {
    VorbisStream* stream = static_cast<VorbisStream*>(data_source);
    return stream->Seek(offset, whence);
}

// Tell function for Vorbis decoder
long tell_func(void* data_source) {
    VorbisStream* stream = static_cast<VorbisStream*>(data_source);
    return stream->Tell();
}

// Close function for Vorbis decoder
int close_func(void* data_source) {
    // Nothing to do here since we don't manage the memory for the data
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
    std::cout << "Created sound buffer of size " << data.samples.size() << " with id " << _buffer_id << std::endl;
#endif
    _is_instanced = true;
}

glm::uint sound_ref::get_id() const
{
    return _buffer_id;
}

audio_data load_compressed_audio_data(const std::vector<char>& audio_stream)
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

std::shared_future<std::shared_ptr<sound_ref>> fetch_sound(const std::filesystem::path& audio_path)
{
    std::promise<std::shared_ptr<sound_ref>>& _promise = detail::promises[audio_path.string()];
    on_audio_locked([&_promise, audio_path] () {
        fetch_file(audio_path, [&_promise](const std::vector<char>& audio_bytes) {
            _promise.set_value(std::make_shared<sound_ref>(load_compressed_audio_data(audio_bytes)));
        });
    });
    return _promise.get_future();
}

void clear_sound_fetches()
{
    detail::promises.clear();
}