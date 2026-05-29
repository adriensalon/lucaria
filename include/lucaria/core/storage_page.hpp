#pragma once

#include <lucaria/core/storage_entity.hpp>

namespace lucaria {
namespace detail {

    // page size

    template <typename PageType, typename Allocator>
    struct object_page_pool_cpu {
        using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<PageType>;
        using allocator_traits = std::allocator_traits<allocator_type>;

        explicit object_page_pool_cpu(const Allocator& alloc = Allocator {})
            : _allocator { alloc }
        {
        }

        ~object_page_pool_cpu()
        {
            clear_memory();
        }

        object_page_pool_cpu(const object_page_pool_cpu&) = delete;
        object_page_pool_cpu& operator=(const object_page_pool_cpu&) = delete;

        PageType* acquire()
        {
            PageType* _ptr = nullptr;

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

        void release(PageType* ptr) noexcept
        {
            if (ptr == nullptr) {
                return;
            }

            // PageType remains constructed
            // caller must have reset its runtime contents
            _free.push_back(ptr);
        }

        void clear_memory() noexcept
        {
            for (PageType* _ptr : _allocated) {
                allocator_traits::destroy(_allocator, _ptr);
                allocator_traits::deallocate(_allocator, _ptr, 1);
            }

            _allocated.clear();
            _free.clear();
        }

    private:
        allocator_type _allocator = {};
        std::vector<PageType*> _allocated = {};
        std::vector<PageType*> _free = {};
    };

    template <typename T>
    struct object_component_readback_gpu;

    template <typename T>
    struct object_component_upload_snapshot_gpu;

    template <typename T>
    struct object_component_download_snapshot_gpu;

    struct object_dirty_range_gpu;

}
}
