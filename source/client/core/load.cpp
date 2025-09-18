#include <lucaria/core/load.hpp>

namespace ozz {
namespace io {

    bool StdStringStreamWrapper::opened() const
    {
        return _stream.good();
    }

    std::size_t StdStringStreamWrapper::Read(void* buffer, std::size_t size)
    {
        _stream.read(static_cast<char*>(buffer), size);
        return _stream.gcount();
    }

    std::size_t StdStringStreamWrapper::Write(const void* buffer, std::size_t size)
    {
        return 0;
    }

    int StdStringStreamWrapper::Tell() const
    {
        return static_cast<int>(_stream.tellg());
    }

    int StdStringStreamWrapper::Seek(int offset, Origin origin)
    {
        std::ios_base::seekdir _direction;
        switch (origin) {
        case ozz::io::Stream::kCurrent:
            _direction = std::ios::cur;
            break;
        case ozz::io::Stream::kEnd:
            _direction = std::ios::end;
            break;
        case ozz::io::Stream::kSet:
            _direction = std::ios::beg;
            break;
        default:
            return -1;
        }
        _stream.seekg(offset, _direction);
        if (_stream.fail()) {
            return -1;
        }
        return static_cast<int>(_stream.tellg());
    }

    std::size_t StdStringStreamWrapper::Size() const
    {
        auto _current = _stream.tellg();
        _stream.seekg(0, std::ios::end);
        auto _size = _stream.tellg();
        _stream.seekg(_current);
        return _size;
    }

}
}