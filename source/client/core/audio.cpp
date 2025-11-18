#include <vorbis/vorbisfile.h>

#include <lucaria/core/audio.hpp>
#include <lucaria/core/error.hpp>

namespace lucaria {
namespace {

    struct vorbis_bytes_stream {

        vorbis_bytes_stream(const std::vector<char>& data)
            : _bytes(data)
            , _position(0)
        {
        }

        std::size_t read(void* ptr, std::size_t size, std::size_t nmemb)
        {
            std::size_t remaining = _bytes.size() - _position;
            std::size_t to_read = std::min(size * nmemb, remaining);
            std::memcpy(ptr, _bytes.data() + _position, to_read);
            _position += to_read;
            return to_read / size; // Return the number of items read
        }

        int seek(ogg_int64_t offset, int whence)
        {
            ogg_int64_t new_position = 0;
            switch (whence) {
            case SEEK_SET:
                new_position = offset;
                break;
            case SEEK_CUR:
                new_position = _position + offset;
                break;
            case SEEK_END:
                new_position = _bytes.size() + offset;
                break;
            default:
                return -1;
            }
            if (new_position < 0 || static_cast<std::size_t>(new_position) > _bytes.size()) {
                return -1;
            }
            _position = new_position;
            return 0;
        }

        long tell() const
        {
            return static_cast<long>(_position);
        }

    private:
        const std::vector<char>& _bytes;
        std::size_t _position;
    };

    static std::size_t read_func(void* ptr, std::size_t size, std::size_t nmemb, void* data_source)
    {
        vorbis_bytes_stream* stream = static_cast<vorbis_bytes_stream*>(data_source);
        return stream->read(ptr, size, nmemb);
    }

    static int seek_func(void* data_source, ogg_int64_t offset, int whence)
    {
        vorbis_bytes_stream* stream = static_cast<vorbis_bytes_stream*>(data_source);
        return stream->seek(offset, whence);
    }

    static long tell_func(void* data_source)
    {
        vorbis_bytes_stream* stream = static_cast<vorbis_bytes_stream*>(data_source);
        return stream->tell();
    }

    static int close_func(void* data_source)
    {
        // nothing to do here since we don't manage the memory for the data
        return 0;
    }

    static void load_data_from_bytes(audio_data& data, const std::vector<char>& data_bytes)
    {
        vorbis_bytes_stream _stream(data_bytes);
        OggVorbis_File _vorbis;
        ov_callbacks _callbacks;
        _callbacks.read_func = read_func;
        _callbacks.seek_func = seek_func;
        _callbacks.tell_func = tell_func;
        _callbacks.close_func = close_func;

        if (ov_open_callbacks(&_stream, &_vorbis, nullptr, 0, _callbacks) != 0) {
            LUCARIA_RUNTIME_ERROR("Failed to open OGG file")
        }

        vorbis_info* _info = ov_info(&_vorbis, -1);
        if (_info->channels != 1) {
            ov_clear(&_vorbis);
            LUCARIA_RUNTIME_ERROR("Failed to open OGG file, only mono files are supported")
        }

        data.sample_rate = _info->rate;
        float** _pcm_channels;
        int _bitstream;
        long _samples;
        while ((_samples = ov_read_float(&_vorbis, &_pcm_channels, 512, &_bitstream)) > 0) {
            for (long i = 0; i < _samples; ++i) {
                data.samples.push_back(_pcm_channels[0][i]);
            }
        }
        if (_samples < 0) {
            LUCARIA_RUNTIME_ERROR("Failed to read OGG file")
        }

        ov_clear(&_vorbis);
    }

}

audio::audio(audio_data&& data)
{
    this->data = std::move(data);
}

audio::audio(const std::vector<char>& data_bytes)
{
    load_data_from_bytes(data, data_bytes);
}

audio::audio(const std::filesystem::path& data_path)
{
    detail::load_bytes(data_path, [this](const std::vector<char>& _data_bytes) {
        load_data_from_bytes(data, _data_bytes);
    });
}

fetched<audio> fetch_audio(const std::filesystem::path& data_path)
{
    std::shared_ptr<std::promise<audio>> _promise = std::make_shared<std::promise<audio>>();
    detail::fetch_bytes(data_path, [_promise](const std::vector<char>& _data_bytes) {
        audio _audio(_data_bytes);
        _promise->set_value(std::move(_audio));
    });

    return fetched<audio>(_promise->get_future());
}

}