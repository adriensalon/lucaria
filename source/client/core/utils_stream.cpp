#include <cstring>

#include <lucaria/core/utils_stream.hpp>

namespace lucaria {
namespace detail {

    bytes_streambuf::bytes_streambuf(const std::vector<char>& data)
    {
        char* begin = const_cast<char*>(data.data());
        char* end = begin + data.size();
        setg(begin, begin, end);
    }

    bytes_stream::bytes_stream(const std::vector<char>& data)
        : std::istream(&_buffer)
        , _buffer(data)
    {
        this->setstate(std::ios::goodbit);
    }

    ozz_bytes_stream::ozz_bytes_stream(const std::vector<char>& data)
        : _bytes(data)
        , _position(0)
    {
    }

    bool ozz_bytes_stream::opened() const
    {
        // the stream is always opened when constructed
        return true;
    }

    std::size_t ozz_bytes_stream::Read(void* buffer, std::size_t size)
    {
        std::size_t remaining = _bytes.size() - _position;
        std::size_t to_read = std::min(size, remaining);
        std::memcpy(buffer, _bytes.data() + _position, to_read);
        _position += to_read;
        return to_read;
    }

    std::size_t ozz_bytes_stream::Write(const void* buffer, std::size_t size)
    {
        // not implemented since this is a read only stream
        return 0;
    }

    int ozz_bytes_stream::Seek(int offset, Origin origin)
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

    int ozz_bytes_stream::Tell() const
    {
        return static_cast<int>(_position);
    }

    std::size_t ozz_bytes_stream::Size() const
    {
        return _bytes.size();
    }

}

}
