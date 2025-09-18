#pragma once

#include <sstream>
#include <vector>

#include <ozz/base/io/stream.h>

namespace ozz {
namespace io {

    class StdStringStreamWrapper : public Stream {
    public:
        explicit StdStringStreamWrapper(std::istringstream& stream)
            : _stream(stream)
        {
        }
        virtual bool opened() const override;
        virtual std::size_t Read(void* buffer, std::size_t size) override;
        virtual std::size_t Write(const void* buffer, std::size_t size) override;
        virtual int Seek(int offset, Origin origin) override;
        virtual int Tell() const override;
        virtual std::size_t Size() const override;

    private:
        std::istringstream& _stream;
    };

}
}

namespace lucaria {

class raw_input_stream_buf : public std::streambuf {
public:
    raw_input_stream_buf(const std::vector<char>& data)
    {
        char* begin = const_cast<char*>(data.data());
        char* end = begin + data.size();
        setg(begin, begin, end);
    }
};

class raw_input_stream : public std::istream {
public:
    raw_input_stream(const std::vector<char>& data)
        : std::istream(&_buffer)
        , _buffer(data)
    {
        this->setstate(std::ios::goodbit);
    }

private:
    raw_input_stream_buf _buffer;
};

class ozz_raw_input_stream : public ozz::io::Stream {
public:
    ozz_raw_input_stream(const std::vector<char>& data)
        : data_(data), position_(0) {}

    ~ozz_raw_input_stream() override = default;

    bool opened() const override {
        return true;  // The stream is always "opened" when constructed
    }

    size_t Read(void* buffer, size_t size) override {
        size_t remaining = data_.size() - position_;
        size_t to_read = std::min(size, remaining);
        std::memcpy(buffer, data_.data() + position_, to_read);
        position_ += to_read;
        return to_read;
    }

    size_t Write(const void* buffer, size_t size) override {
        // Not implemented since this is a read-only stream
        return 0;
    }

    int Seek(int offset, Origin origin) override {
        int new_position = 0;
        switch (origin) {
            case kSet:
                new_position = offset;
                break;
            case kCurrent:
                new_position = static_cast<int>(position_ + offset);
                break;
            case kEnd:
                new_position = static_cast<int>(data_.size() + offset);
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

    int Tell() const override {
        return static_cast<int>(position_);
    }
    
    size_t Size() const override {
        return data_.size();
    }

private:
    const std::vector<char>& data_;
    size_t position_;
};

}
