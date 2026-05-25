#include <lucaria/core/utils_error.hpp>
#include <lucaria/core/utils_owning.hpp>

namespace lucaria {
namespace detail {

    flag_owning::flag_owning(flag_owning&& other) noexcept
        : _is_owning(std::exchange(other._is_owning, false))
    {
    }

    flag_owning& flag_owning::operator=(flag_owning&& other) noexcept
    {
        if (this != &other) {
            if (_is_owning) {
                LUCARIA_DEBUG_ERROR("Object already owning resources")
            }

            _is_owning = std::exchange(other._is_owning, false);
        }

        return *this;
    }

    void flag_owning::emplace() noexcept
    {
        if (_is_owning) {
            LUCARIA_DEBUG_ERROR("Object already owning resources")
        }

		_is_owning = true;
    }

    bool flag_owning::owns() const noexcept
    {
        return _is_owning;
    }

}
}