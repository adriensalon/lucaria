#pragma once

#include <entt/entt.hpp>

#include <lucaria/core/scenes_entity.hpp>
#include <lucaria/core/app_error.hpp>

namespace lucaria {
namespace detail {

    template <typename Page, typename Allocator>
    struct storage_page_pool_memory {
        using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<Page>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        explicit storage_page_pool_memory(const Allocator& alloc = Allocator {})
            : _allocator { alloc }
        {
        }

        ~storage_page_pool_memory()
        {
            clear_memory();
        }

        storage_page_pool_memory(const storage_page_pool_memory&) = delete;
        storage_page_pool_memory& operator=(const storage_page_pool_memory&) = delete;

        Page* acquire()
        {
            Page* _ptr = nullptr;

            if (!_free.empty()) {
                _ptr = _free.back();
                _free.pop_back();

            } else {
                _ptr = allocator_traits::allocate(_allocator, 1);
                allocator_traits::construct(_allocator, _ptr);
                _allocated.push_back(_ptr);
            }

            return _ptr;
        }

        void release(Page* ptr) noexcept
        {
            if (ptr == nullptr) {
                return;
            }

            // Page remains constructed
            // caller must have reset its runtime contents
            _free.push_back(ptr);
        }

        void clear_memory() noexcept
        {
            for (Page* _ptr : _allocated) {
                allocator_traits::destroy(_allocator, _ptr);
                allocator_traits::deallocate(_allocator, _ptr, 1);
            }

            _allocated.clear();
            _free.clear();
        }

    private:
        allocator_type _allocator = {};
        std::vector<Page*> _allocated = {};
        std::vector<Page*> _free = {};
    };

    template <typename T, typename Entity, typename ComputeAllocator>
    struct storage_page_compute {
        using value_type = T;
        using entity_type = Entity;
        using compute_allocator_type = ComputeAllocator;
        using allocation_type = typename compute_allocator_type::allocation_type;

        static constexpr std::uint32_t page_size = compute_allocator_type::page_size;

        entity_type entities[page_size] = {};
        allocation_type allocation = {};
        compute_allocator_type* compute_allocator = nullptr;
        std::uint32_t count = {};

        void bind_compute_allocator(compute_allocator_type& allocator)
        {
            compute_allocator = &allocator;
            allocation = compute_allocator->allocate_page();
        }

        void release_compute_allocation() noexcept
        {
            if (compute_allocator != nullptr) {
                compute_allocator->release_page(allocation);
                compute_allocator = nullptr;
            }
            count = 0u;
        }

        void reset() noexcept
        {
            count = 0u;
        }

        void set_count(std::uint32_t value) noexcept
        {
            count = value;
        }

        [[nodiscard]] std::uint32_t size() const noexcept
        {
            return count;
        }

        template <typename... Args>
        void emplace(std::uint32_t offset, entity_type entity, Args&&... args)
        {
            LUCARIA_DEBUG_ASSERT(compute_allocator != nullptr, "Compute page has no allocator");
            LUCARIA_DEBUG_ASSERT(offset < page_size, "Compute page offset out of range");

            entities[offset] = entity;
            compute_allocator->construct(allocation, offset, std::forward<Args>(args)...);

            if (offset >= count) {
                count = offset + 1u;
            }
        }

        void destroy(std::uint32_t offset) noexcept
        {
            LUCARIA_DEBUG_ASSERT(compute_allocator != nullptr, "Compute page has no allocator");
            LUCARIA_DEBUG_ASSERT(offset < count, "Compute page destroy offset out of range");
            compute_allocator->destroy(allocation, offset);
        }

        void move_construct(std::uint32_t dst_offset, std::uint32_t src_offset)
        {
            LUCARIA_DEBUG_ASSERT(compute_allocator != nullptr, "Compute page has no allocator");
            LUCARIA_DEBUG_ASSERT(dst_offset < page_size, "Compute page move dst out of range");
            LUCARIA_DEBUG_ASSERT(src_offset < count, "Compute page move src out of range");

            entities[dst_offset] = entities[src_offset];
            compute_allocator->move_construct(allocation, dst_offset, allocation, src_offset);
        }

        [[nodiscard]] entity_type entity(std::uint32_t offset) const noexcept
        {
            LUCARIA_DEBUG_ASSERT(offset < count, "Compute page entity offset out of range");
            return entities[offset];
        }

        [[nodiscard]] std::uint32_t backend_index(std::uint32_t offset) const noexcept
        {
            LUCARIA_DEBUG_ASSERT(compute_allocator != nullptr, "Compute page has no allocator");
            LUCARIA_DEBUG_ASSERT(offset < count, "Compute page backend index offset out of range");
            return compute_allocator->backend_index(allocation, offset);
        }
    };

    template <typename Page, typename Allocator, typename ComputeAllocator>
    struct storage_page_pool_compute {
        using page_type = Page;
        using compute_allocator_type = ComputeAllocator;
        using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<page_type>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        storage_page_pool_compute(const Allocator& alloc, compute_allocator_type& compute_allocator)
            : _allocator { alloc }
            , _compute_allocator { &compute_allocator }
        {
        }

        ~storage_page_pool_compute()
        {
            clear_memory();
        }

        storage_page_pool_compute(const storage_page_pool_compute&) = delete;
        storage_page_pool_compute& operator=(const storage_page_pool_compute&) = delete;

        page_type* acquire()
        {
            page_type* _ptr = nullptr;

            if (!_free.empty()) {
                _ptr = _free.back();
                _free.pop_back();
                _ptr->reset();
                return _ptr;
            }

            _ptr = allocator_traits::allocate(_allocator, 1);
            allocator_traits::construct(_allocator, _ptr);
            _ptr->bind_compute_allocator(*_compute_allocator);
            _allocated.push_back(_ptr);
            return _ptr;
        }

        void release(page_type* ptr) noexcept
        {
            if (ptr == nullptr) {
                return;
            }

            ptr->reset();
            _free.push_back(ptr);
        }

        void clear_memory() noexcept
        {
            for (page_type* _ptr : _allocated) {
                _ptr->release_compute_allocation();
                allocator_traits::destroy(_allocator, _ptr);
                allocator_traits::deallocate(_allocator, _ptr, 1);
            }

            _allocated.clear();
            _free.clear();
        }

    private:
        allocator_type _allocator = {};
        compute_allocator_type* _compute_allocator = nullptr;
        std::vector<page_type*> _allocated = {};
        std::vector<page_type*> _free = {};
    };

}
}
