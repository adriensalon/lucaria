#pragma once

#include <istream>
#include <vector>

#include <ozz/base/io/archive.h>
#include <ozz/base/io/stream.h>
#include <ozz/base/memory/allocator.h>

namespace lucaria {
namespace detail {

    struct bytes_streambuf : public std::streambuf {
        bytes_streambuf(const std::vector<char>& data);
    };

    struct bytes_stream : public std::istream {
        bytes_stream(const std::vector<char>& data);

    private:
        bytes_streambuf _buffer;
    };

    struct ozz_bytes_stream : public ozz::io::Stream {
        ozz_bytes_stream(const std::vector<char>& data);
        ~ozz_bytes_stream() override = default;

        bool opened() const override;
        std::size_t Read(void* buffer, std::size_t size) override;
        std::size_t Write(const void* buffer, std::size_t size) override;
        int Seek(int offset, Origin origin) override;
        int Tell() const override;
        std::size_t Size() const override;

    private:
        const std::vector<char>& _bytes;
        std::size_t _position;
    };

}
}