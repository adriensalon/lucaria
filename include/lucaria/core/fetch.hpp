#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <optional>
#include <type_traits>
#include <vector>

#include <ozz/base/io/stream.h>

#include <lucaria/core/error.hpp>

namespace lucaria {

/// @brief Represents a fetched value
/// @tparam value_t type being fetched
template <typename value_t>
struct fetched {

    fetched(std::future<value_t>&& future)
    {
        std::shared_ptr<std::future<value_t>> _shared_future = std::make_shared<std::future<value_t>>(std::move(future));

        _poll = [_shared_future]() -> bool {
            if (!_shared_future->valid()) {
                return false;
            }
            return _shared_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        };

        _get = [_shared_future]() -> value_t {
            return _shared_future->get();
        };
    }

    template <typename U, typename Then, typename = std::enable_if_t<std::is_invocable_r_v<value_t, const Then&, const U&>>>
    fetched(std::future<U>&& future, const Then& then_fn)
    {
        std::shared_ptr<std::future<U>> _shared_intermediate_future = std::make_shared<std::future<U>>(std::move(future));
        std::shared_ptr<std::decay_t<Then>> _shared_decayed_then = std::make_shared<std::decay_t<Then>>(then_fn);

        _poll = [_shared_intermediate_future]() -> bool {
            if (!_shared_intermediate_future->valid()) {
                return false;
            }
            return _shared_intermediate_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        };

        _get = [_shared_intermediate_future, _shared_decayed_then]() -> value_t {
            const U _intermediate_value = _shared_intermediate_future->get();
            return std::invoke(*_shared_decayed_then, _intermediate_value);
        };
    }

    /// @brief
    /// @return
    [[nodiscard]] bool has_value() const
    {
        if (_cache) {
            return true;
        }

        if (_poll && _poll()) {
            _cache = std::move(_get());
            _poll = nullptr;
            _get = nullptr;
            return true;
        }

        return false;
    }

    /// @brief
    /// @return
    [[nodiscard]] value_t& value()
    {
        if (!has_value()) {
            LUCARIA_RUNTIME_ERROR("Failed to get fetched value&, please check has_value() before trying to access it")
        }
        return _cache.value();
    }

    /// @brief
    /// @return
    [[nodiscard]] const value_t& value() const
    {
        if (!has_value()) {
            LUCARIA_RUNTIME_ERROR("Failed to get fetched const value&, please check has_value() before trying to access it")
        }
        return _cache.value();
    }

    /// @brief
    [[nodiscard]] explicit operator bool() const { return has_value(); }

private:
    mutable std::function<bool()> _poll;
    mutable std::function<value_t()> _get;
    mutable std::optional<value_t> _cache;
};

[[nodiscard]] int get_fetches_waiting();

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

    template <typename value_t>
    struct fetched_container {

        void emplace(value_t& obj)
        {
            _ptr = &obj;
        }

        void emplace(fetched<value_t>& fut, const std::function<void()>& callback = nullptr)
        {
            _fut = &fut;
            _callback = callback;
        }

        bool has_value() const
        {
            if (_ptr) {
                return true;
            }
            if (_fut && _fut->has_value()) {
                if (_callback && !_is_callback_invoked) {
                    _is_callback_invoked = true;
                    _callback();
                    _callback = nullptr;
                }
                return true;
            }
            return false;
        }

        value_t& value()
        {
            return _ptr ? *_ptr : _fut->value();
        }

        const value_t& value() const
        {
            return _ptr ? *_ptr : _fut->value();
        }

        explicit operator bool() const { return has_value(); }

    private:
        value_t* _ptr = nullptr;
        fetched<value_t>* _fut = nullptr;
        mutable std::function<void()> _callback = nullptr;
        mutable bool _is_callback_invoked = false;
    };

    void load_bytes(
        const std::filesystem::path& file_path,
        const std::function<void(const std::vector<char>&)>& callback);

    void fetch_bytes(
        const std::filesystem::path& file_path,
        const std::function<void(const std::vector<char>&)>& callback,
        const bool persist = true);

    void fetch_bytes(
        const std::vector<std::filesystem::path>& file_paths,
        const std::function<void(const std::vector<std::vector<char>>&)>& callback,
        const bool persist = true);

}
}
