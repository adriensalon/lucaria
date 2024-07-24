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

class VectorStreamBuf : public std::streambuf {
public:
    VectorStreamBuf(const std::vector<char>& data) {
        // Set the input buffer pointers to the start and end of the vector
        char* begin = const_cast<char*>(data.data());
        char* end = begin + data.size();
        setg(begin, begin, end);
    }
};

class VectorInputStream : public std::istream {
public:
    VectorInputStream(const std::vector<char>& data)
        : std::istream(&buffer), buffer(data) {
        // Ensure the stream state is good
        this->setstate(std::ios::goodbit);
    }

private:
    VectorStreamBuf buffer;
};