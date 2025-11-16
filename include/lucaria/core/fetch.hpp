#pragma once

#include <filesystem>
#include <functional>
#include <future>
#include <optional>
#include <type_traits>
#include <vector>

#include <ozz/base/io/stream.h>

#include <lucaria/core/error.hpp>

namespace lucaria {

/// @brief Represents a fetched value
/// @tparam T type being fetched
template <typename T>
struct fetched {

    /// @brief Creates a fetched object from an existing std::future<T>
    /// @param future the future to create from
    fetched(std::future<T>&& future)
    {
        std::shared_ptr<std::future<T>> _shared_future = std::make_shared<std::future<T>>(std::move(future));

        _poll = [_shared_future]() -> bool {
            if (!_shared_future->valid()) {
                return false;
            }
            return _shared_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        };

        _get = [_shared_future]() -> T {
            return _shared_future->get();
        };
    }

    /// @brief Creates a fetched object from an existing std::future<U> and a continuation from U to T
    /// @tparam U intermediate type to continuate
    /// @tparam Then function type for the continuation
    /// @param future the future to create from
    /// @param then the continuation
    template <typename U, typename Then, typename = std::enable_if_t<std::is_invocable_r_v<T, const Then&, const U&>>>
    fetched(std::future<U>&& future, const Then& then)
    {
        std::shared_ptr<std::future<U>> _shared_intermediate_future = std::make_shared<std::future<U>>(std::move(future));
        std::shared_ptr<std::decay_t<Then>> _shared_decayed_then = std::make_shared<std::decay_t<Then>>(then);

        _poll = [_shared_intermediate_future]() -> bool {
            if (!_shared_intermediate_future->valid()) {
                return false;
            }
            return _shared_intermediate_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        };

        _get = [_shared_intermediate_future, _shared_decayed_then]() -> T {
            const U _intermediate_value = _shared_intermediate_future->get();
            return std::invoke(*_shared_decayed_then, _intermediate_value);
        };
    }

    /// @brief Checks if the underlying std::future<T> has yet a value or is still computing
    /// @return true if the value is available
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

    /// @brief Gets the available value held by the underlying std::future<T>.
    /// @throws throws a lucaria::runtime_error if the std::future result is not available yet
    /// @return the available value
    [[nodiscard]] T& value()
    {
        if (!has_value()) {
            LUCARIA_RUNTIME_ERROR("Failed to get fetched value&, please check has_value() before trying to access it")
        }
        return _cache.value();
    }

    /// @brief Gets the available value held by the underlying std::future<T>.
    /// @throws throws a lucaria::runtime_error if the std::future result is not available yet
    /// @return the available value
    [[nodiscard]] const T& value() const
    {
        if (!has_value()) {
            LUCARIA_RUNTIME_ERROR("Failed to get fetched const value&, please check has_value() before trying to access it")
        }
        return _cache.value();
    }

    /// @brief Conversion operator for the has_value member function
    [[nodiscard]] explicit operator bool() const
    {
        return has_value();
    }

private:
    mutable std::function<bool()> _poll;
    mutable std::function<T()> _get;
    mutable std::optional<T> _cache;
};

/// @brief Gets the current count of fetched objects that still have a std::future waiting
/// @return the current count
[[nodiscard]] std::size_t get_fetches_waiting();

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

    template <typename T>
    struct fetched_container {

        void emplace(T& obj)
        {
            _ptr = &obj;
        }

        void emplace(fetched<T>& fut, const std::function<void()>& callback = nullptr)
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

        T& value()
        {
            return _ptr ? *_ptr : _fut->value();
        }

        const T& value() const
        {
            return _ptr ? *_ptr : _fut->value();
        }

        explicit operator bool() const { return has_value(); }

    private:
        T* _ptr = nullptr;
        fetched<T>* _fut = nullptr;
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
