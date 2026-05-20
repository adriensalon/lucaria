#pragma once

#include <utility>

namespace lucaria {
namespace detail {

	struct owning_flag {
		owning_flag() = default;
		owning_flag(const owning_flag&) = delete;
		owning_flag& operator=(const owning_flag&) = delete;
		owning_flag(owning_flag&& other) noexcept;
		owning_flag& operator=(owning_flag&& other) noexcept;

		void emplace() noexcept;
		[[nodiscard]] bool owns() const noexcept;

	private:
		bool _is_owning = false;
	};

}
}