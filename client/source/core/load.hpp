#pragma once

#include <sstream>

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

