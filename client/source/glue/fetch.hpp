#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
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

template <typename value_t>
bool get_is_future_ready(const std::future<value_t>& future)
{
    return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

template <typename value_t>
bool get_is_future_ready(const std::shared_future<value_t>& future)
{
    return future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

using fetch_callback = std::function<void(std::istringstream&)>;
using multiple_fetch_callback = std::function<void(std::size_t, std::size_t, std::istringstream&)>;

std::size_t compute_hash_files(const std::vector<std::filesystem::path>& files);
void fetch_file(const std::filesystem::path& file, const fetch_callback& callback, const bool persist = true);
void fetch_files(const std::vector<std::filesystem::path>& files, const multiple_fetch_callback& callback, const bool persist = true);
std::size_t get_fetches_completed();
std::size_t get_fetches_failed();
std::size_t get_fetches_total();
void reset_fetch_counters();
