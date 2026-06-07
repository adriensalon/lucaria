#pragma once

#include <istream>
#include <vector>

#include <ozz/base/io/archive.h>
#include <ozz/base/io/stream.h>
#include <ozz/base/memory/allocator.h>

namespace lucaria {
namespace detail {

    struct assets_bytes_streambuf : public std::streambuf {
        assets_bytes_streambuf(const std::vector<char>& data);
    };

    struct assets_bytes_stream : public std::istream {
        assets_bytes_stream(const std::vector<char>& data);

    private:
        assets_bytes_streambuf _buffer;
    };

    struct assets_bytes_stream_ozz : public ozz::io::Stream {
        assets_bytes_stream_ozz(const std::vector<char>& data);
        ~assets_bytes_stream_ozz() override = default;

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