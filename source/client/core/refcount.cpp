#include <utility>

#include <lucaria/core/refcount.hpp>

namespace lucaria {
namespace detail {

    refcount_flag::refcount_flag(const refcount_flag& other) noexcept
        : _count(other._count)
    {
        _retain();
    }

    refcount_flag& refcount_flag::operator=(const refcount_flag& other) noexcept
    {
        if (this != &other) {
            _release();
            _count = other._count;
            _retain();
        }

        return *this;
    }

    refcount_flag::refcount_flag(refcount_flag&& other) noexcept
        : _count(std::exchange(other._count, nullptr))
    {
    }

    refcount_flag& refcount_flag::operator=(refcount_flag&& other) noexcept
    {
        if (this != &other) {
            _release();
            _count = std::exchange(other._count, nullptr);
        }

        return *this;
    }

    refcount_flag::~refcount_flag()
    {
        _release();
    }

    void refcount_flag::emplace()
    {
        _release();
        _count = new uint32(1);
    }

    bool refcount_flag::owns() const noexcept
    {
        return _count != nullptr;
    }

    bool refcount_flag::is_last_owner() const noexcept
    {
        return _count && *_count == 1;
    }

    uint32 refcount_flag::use_count() const noexcept
    {
        return _count ? *_count : 0;
    }

    void refcount_flag::_retain() noexcept
    {
        if (_count) {
            ++*_count;
        }
    }

    void refcount_flag::_release() noexcept
    {
        if (_count) {
            --*_count;

            if (*_count == 0) {
                delete _count;
            }

            _count = nullptr;
        }
    }

}
}