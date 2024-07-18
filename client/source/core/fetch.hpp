#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <vector>
#include <unordered_map>

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





inline std::unordered_map<std::uintptr_t, std::function<bool()>> fetch_container_updaters = {};

template <typename value_t>
struct fetch_container {
    fetch_container() = default;
    fetch_container(const fetch_container& other) = delete;
    fetch_container& operator=(const fetch_container& other) = delete;
    fetch_container(fetch_container&& other) = default;
    fetch_container& operator=(fetch_container&& other) = default;

    void emplace(const std::shared_future<std::shared_ptr<value_t>>& fetched, const std::function<void()>& callback = nullptr)
    {
        _fetched = fetched;
        _callback = callback;
        fetch_container_updaters[reinterpret_cast<std::uintptr_t>(static_cast<void*>(this))] = [this] () {
            bool _must_erase = false;
            if (_fetched.has_value()) {
                std::shared_future<std::shared_ptr<value_t>>& _future_value = _fetched.value();
                if (get_is_future_ready<std::shared_ptr<value_t>>(_future_value)) {
                    _value = _future_value.get();
                    if (_callback) {
                        _callback();
                    }
                    _fetched = std::nullopt;
                    return true;
                }
            }
            return false;
        };
    }

    bool has_value() const
    {
        return _value.operator bool();
    }

    value_t& value()
    {
        return *(_value.get());
    }

    const value_t& value() const
    {
        return *(_value.get());
    }


private:
    std::optional<std::shared_future<std::shared_ptr<value_t>>> _fetched = std::nullopt;
    std::shared_ptr<value_t> _value = nullptr;
    std::function<void()> _callback = nullptr;
};

inline void wait_fetched_containers()
{
    std::vector<std::uintptr_t> _to_erase = {};
    for (std::pair<const std::uintptr_t, std::function<bool()>>& _pair : fetch_container_updaters) {
        if (_pair.second()) {
            _to_erase.emplace_back(_pair.first);
        }
    }
    for (const std::uintptr_t _element : _to_erase) {
        fetch_container_updaters.erase(_element);
    }
}
