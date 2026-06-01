#pragma once

#include <lucaria/core/storage_entity.hpp>

namespace lucaria {
namespace detail {

	template <typename T>
	using storage_component_allocator_memory = std::allocator<T>;

    template <typename T, typename Entity, std::uint32_t PageSize = 1024u>
    struct storage_component_allocator_compute_DEBUG {
        using value_type = T;
        using entity_type = Entity;

        struct allocation_type {
            std::uint32_t page = {};
            std::uint32_t base_index = {};
            value_type* components = nullptr;
        };

        static constexpr std::uint32_t page_size = PageSize;

        storage_component_allocator_compute_DEBUG() = default;

        allocation_type allocate_page()
        {
            const std::uint32_t _page = _page_count++;
            value_type* _components = static_cast<value_type*>(::operator new(sizeof(value_type) * page_size));
            return allocation_type {
                .page = _page,
                .base_index = _page * page_size,
                .components = _components
            };
        }

        void release_page(allocation_type allocation) noexcept
        {
            ::operator delete(allocation.components);
        }

        template <typename... Args>
        void construct(allocation_type allocation, std::uint32_t offset, Args&&... args)
        {
            value_type* _ptr = allocation.components + offset;
            ::new (static_cast<void*>(_ptr)) value_type(std::forward<Args>(args)...);
        }

        void destroy(allocation_type allocation, std::uint32_t offset) noexcept
        {
            (allocation.components + offset)->~value_type();
        }

        void move_construct(allocation_type dst_allocation, std::uint32_t dst_offset, allocation_type src_allocation, std::uint32_t src_offset)
        {
            value_type* _dst = dst_allocation.components + dst_offset;
            value_type* _src = src_allocation.components + src_offset;
            ::new (static_cast<void*>(_dst)) value_type(std::move(*_src));
            _src->~value_type();
        }

        [[nodiscard]] value_type* components(allocation_type allocation) noexcept
        {
            return allocation.components;
        }

        [[nodiscard]] const value_type* components(allocation_type allocation) const noexcept
        {
            return allocation.components;
        }

        [[nodiscard]] std::uint32_t backend_index(allocation_type allocation, std::uint32_t offset) const noexcept
        {
            return allocation.base_index + offset;
        }

    private:
        std::uint32_t _page_count = {};
    };

	// TODO storage_component_allocator_opengl

}
}
