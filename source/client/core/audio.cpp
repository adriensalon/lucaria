#include <cstring>

#include <vorbis/vorbisfile.h>

#include <lucaria/core/audio.hpp>
#include <lucaria/core/error.hpp>
#include <lucaria/core/database.hpp>
#include <lucaria/core/fetch.hpp>

namespace lucaria {
namespace detail {
	
    namespace {

        struct _vorbis_bytes_stream {

            _vorbis_bytes_stream(const std::vector<char>& data)
                : _bytes(data)
                , _position(0)
            {
            }

            std::size_t read(void* ptr, std::size_t size, std::size_t nmemb)
            {
                const std::size_t _remaining = _bytes.size() - _position;
                const std::size_t _to_read = std::min(size * nmemb, _remaining);
                std::memcpy(ptr, _bytes.data() + _position, _to_read);
                _position += _to_read;
                return _to_read / size;
            }

            int seek(ogg_int64_t offset, int whence)
            {
                ogg_int64_t _new_position = 0;
                switch (whence) {
                case SEEK_SET:
                    _new_position = offset;
                    break;
                case SEEK_CUR:
                    _new_position = _position + offset;
                    break;
                case SEEK_END:
                    _new_position = _bytes.size() + offset;
                    break;
                default:
                    return -1;
                }
                if (_new_position < 0 || static_cast<std::size_t>(_new_position) > _bytes.size()) {
                    return -1;
                }
                _position = _new_position;
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

        static std::size_t _vorbis_read_callback(void* ptr, std::size_t size, std::size_t nmemb, void* data_source)
        {
            _vorbis_bytes_stream* _stream = static_cast<_vorbis_bytes_stream*>(data_source);
            return _stream->read(ptr, size, nmemb);
        }

        static int _vorbis_seek_callback(void* data_source, ogg_int64_t offset, int whence)
        {
            _vorbis_bytes_stream* _stream = static_cast<_vorbis_bytes_stream*>(data_source);
            return _stream->seek(offset, whence);
        }

        static long _vorbis_tell_callback(void* data_source)
        {
            _vorbis_bytes_stream* _stream = static_cast<_vorbis_bytes_stream*>(data_source);
            return _stream->tell();
        }

        static int _vorbis_close_callback(void* data_source)
        {
            return 0;
        }

        static void _load_audio_bytes(audio_data& data, const std::vector<char>& bytes)
        {
            _vorbis_bytes_stream _stream(bytes);
            OggVorbis_File _vorbis;
            ov_callbacks _callbacks;
            _callbacks.read_func = _vorbis_read_callback;
            _callbacks.seek_func = _vorbis_seek_callback;
            _callbacks.tell_func = _vorbis_tell_callback;
            _callbacks.close_func = _vorbis_close_callback;

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

        static async_container<audio_implementation> _fetch_audio_async(const std::filesystem::path& path)
        {
            std::shared_ptr<std::promise<audio_implementation>> _promise = std::make_shared<std::promise<audio_implementation>>();
            fetch_bytes(path, [_promise](const std::vector<char>& _data_bytes) {
        audio_implementation _audio(_data_bytes);
        _promise->set_value(std::move(_audio)); }, true);

            return async_container<audio_implementation>(_promise->get_future());
        }
    }

    audio_implementation::audio_implementation(const std::vector<char>& bytes)
    {
        _load_audio_bytes(data, bytes);
    }

    audio_implementation::audio_implementation(audio_data&& data)
        : data(std::move(data))
    {
    }

}

audio_object::~audio_object()
{
	// TODO
}

audio_object audio_object::fetch(const std::filesystem::path& path)
{
    detail::resource_container<detail::audio_implementation>* _resource = detail::engine_resources().audios.get_or_create_by_path(path, [&] {
        return detail::_fetch_audio_async(path);
    });

    return audio_object { _resource };
}

bool audio_object::has_value() const
{
    return _resource && _resource->is_ready();
}

audio_object::operator bool() const
{
    return has_value();
}

audio_object::audio_object(detail::resource_container<detail::audio_implementation>* resource)
    : _resource(resource)
{
}

}