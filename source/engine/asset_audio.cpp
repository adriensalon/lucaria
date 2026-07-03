#include <cstring>

#include <cereal/archives/portable_binary.hpp>
#include <vorbis/vorbisfile.h>

#include <lucaria/core/assets_stream.hpp>
#include <lucaria/core/manager_assets.hpp>
#include <lucaria/engine/context_serialize.hpp>
#include <lucaria/engine/asset_audio.hpp>

namespace lucaria {
namespace detail {

    std::filesystem::path resolve_profile(
        manager_assets& object,
        const std::filesystem::path& path,
        const std::optional<data_audio_profile> profile)
    {
        if (!profile || profile == data_audio_profile::ogg_vorbis) {
            std::filesystem::path _ogg_path = path;
            _ogg_path = _ogg_path.replace_extension().string() + ".ogg.bin";
            if (std::filesystem::exists(object.resolve_fetch_path(_ogg_path))) {
                return _ogg_path;
            }
        }
        return path;
    }

    namespace {

        struct _vorbis_bytes_stream {

            _vorbis_bytes_stream(const std::vector<uint8>& data)
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
            const std::vector<uint8>& _bytes;
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

        static void _append_float32_bytes(std::vector<uint8>& bytes, const float32 value)
        {
            const uint8* _value_bytes = reinterpret_cast<const uint8*>(&value);
            bytes.insert(bytes.end(), _value_bytes, _value_bytes + sizeof(float32));
        }

        static void _decode_ogg_vorbis(data_audio& data)
        {
            _vorbis_bytes_stream _stream(data.samples);
            OggVorbis_File _vorbis;
            ov_callbacks _callbacks;
            _callbacks.read_func = _vorbis_read_callback;
            _callbacks.seek_func = _vorbis_seek_callback;
            _callbacks.tell_func = _vorbis_tell_callback;
            _callbacks.close_func = _vorbis_close_callback;
            if (ov_open_callbacks(&_stream, &_vorbis, nullptr, 0, _callbacks) != 0) {
                LUCARIA_DEBUG_ERROR("Failed to open OGG file")
            }
            vorbis_info* _info = ov_info(&_vorbis, -1);
            if (_info->channels != 1) {
                ov_clear(&_vorbis);
                LUCARIA_DEBUG_ERROR("Failed to open OGG file, only mono files are supported")
            }

            std::vector<uint8> _decoded_samples;
            data.sample_rate = _info->rate;
            data.channels = _info->channels;
            data.count = 0;

            float** _pcm_channels;
            int _bitstream;
            long _samples;
            while ((_samples = ov_read_float(&_vorbis, &_pcm_channels, 512, &_bitstream)) > 0) {
                for (long i = 0; i < _samples; ++i) {
                    _append_float32_bytes(_decoded_samples, _pcm_channels[0][i]);
                }
                data.count += static_cast<uint32>(_samples);
            }
            if (_samples < 0) {
                LUCARIA_DEBUG_ERROR("Failed to read OGG file")
            }
            ov_clear(&_vorbis);

            data.profile = data_audio_profile::float32;
            data.samples = std::move(_decoded_samples);
        }

        static void _normalize(data_audio& data)
        {
            if (data.profile == data_audio_profile::ogg_vorbis) {
                _decode_ogg_vorbis(data);
            }
            if (data.profile != data_audio_profile::float32) {
                LUCARIA_DEBUG_ERROR("Unsupported audio profile")
            }
            if (data.channels != 1) {
                LUCARIA_DEBUG_ERROR("Only mono audio is supported")
            }
        }
    }

    asset_audio::asset_audio(const std::vector<char>& bytes)
        : origin(object_audio_origin::path)
    {
        assets_bytes_stream _stream(bytes);
        cereal::PortableBinaryInputArchive _archive(_stream);
        _archive(data);
        _normalize(data);
    }

    asset_audio::asset_audio(data_audio&& data)
        : origin(object_audio_origin::data)
        , data(std::move(data))
    {
        _normalize(this->data);
    }

    void asset_audio::save(context_save_storage& context) const
    {
        context.field("origin", origin);
        if (origin == object_audio_origin::path) {
            context.field("origin_path", origin_path);
        }
        if (origin == object_audio_origin::data) {
            context.field("origin_data", data);
        }
    }

    void asset_audio::load(context_load_storage& context)
    {
        context.field("origin", origin);
        if (origin == object_audio_origin::path) {
            context.field("origin_path", origin_path);
            const std::filesystem::path _path = origin_path;
            const std::filesystem::path _resolved_path = resolve_profile(context.objects, _path, std::optional<data_audio_profile> {});
            context.fetch_worker(_resolved_path, [this, _path](const std::vector<char>& bytes) {
                *this = asset_audio(bytes);
                origin_path = _path;
            });
        }
        if (origin == object_audio_origin::data) {
            context.field("origin_data", data);
            _normalize(data);
        }
    }

}
}
