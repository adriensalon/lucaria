#include <cstring>

#include <lucaria/core/assets_stream.hpp>

namespace lucaria {
namespace detail {

    assets_bytes_streambuf::assets_bytes_streambuf(const std::vector<char>& data)
    {
        char* begin = const_cast<char*>(data.data());
        char* end = begin + data.size();
        setg(begin, begin, end);
    }

    assets_bytes_stream::assets_bytes_stream(const std::vector<char>& data)
        : std::istream(&_buffer)
        , _buffer(data)
    {
        this->setstate(std::ios::goodbit);
    }

    assets_bytes_stream_ozz::assets_bytes_stream_ozz(const std::vector<char>& data)
        : _bytes(data)
        , _position(0)
    {
    }

    bool assets_bytes_stream_ozz::opened() const
    {
        // the stream is always opened when constructed
        return true;
    }

    std::size_t assets_bytes_stream_ozz::Read(void* buffer, std::size_t size)
    {
        std::size_t remaining = _bytes.size() - _position;
        std::size_t to_read = std::min(size, remaining);
        std::memcpy(buffer, _bytes.data() + _position, to_read);
        _position += to_read;
        return to_read;
    }

    std::size_t assets_bytes_stream_ozz::Write(const void* buffer, std::size_t size)
    {
        // not implemented since this is a read only stream
        return 0;
    }

    int assets_bytes_stream_ozz::Seek(int offset, Origin origin)
    {
        int new_position = 0;
        switch (origin) {
        case kSet:
            new_position = offset;
            break;
        case kCurrent:
            new_position = static_cast<int>(_position + offset);
            break;
        case kEnd:
            new_position = static_cast<int>(_bytes.size() + offset);
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

    int assets_bytes_stream_ozz::Tell() const
    {
        return static_cast<int>(_position);
    }

    std::size_t assets_bytes_stream_ozz::Size() const
    {
        return _bytes.size();
    }

}

}
