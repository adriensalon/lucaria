#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

namespace lucaria {
namespace detail {

    template <typename T, std::size_t PageSize = 256>
    class container_paged_vector {
        static_assert(PageSize > 0, "PageSize must be greater than zero.");
        static_assert(!std::is_reference_v<T>, "container_paged_vector<T> cannot store references.");
        static_assert(!std::is_void_v<T>, "container_paged_vector<T> cannot store void.");
        static_assert(sizeof(T) > 0, "container_paged_vector<T> requires a complete type.");

    private:
        struct slot;

        union slot_data {
            slot_data() noexcept { }
            ~slot_data() noexcept { }

            alignas(T) unsigned char storage[sizeof(T)];
            slot* next_free;
        };

        struct slot {
            slot_data data = {};
            bool alive = false;

            T* value() noexcept
            {
                return std::launder(reinterpret_cast<T*>(data.storage));
            }

            const T* value() const noexcept
            {
                return std::launder(reinterpret_cast<const T*>(data.storage));
            }

            static slot* from_value(T* object) noexcept
            {
                static_assert(std::is_standard_layout_v<slot>,
                    "container_paged_vector slot must remain standard-layout for offsetof.");

                auto* bytes = reinterpret_cast<unsigned char*>(object);

                return reinterpret_cast<slot*>(
                    bytes - offsetof(slot, data));
            }
        };

        static_assert(std::is_standard_layout_v<slot>,
            "container_paged_vector slot must be standard-layout.");

        struct page {
            slot slots[PageSize] = {};
        };

    public:
        container_paged_vector() = default;

        container_paged_vector(const container_paged_vector&) = delete;
        container_paged_vector& operator=(const container_paged_vector&) = delete;

        container_paged_vector(container_paged_vector&&) noexcept = default;
        container_paged_vector& operator=(container_paged_vector&&) noexcept = default;

        ~container_paged_vector()
        {
            clear();
        }

        template <typename... Args>
        T* emplace(Args&&... args)
        {
            slot* target = nullptr;

            if (_free_list) {
                target = _free_list;
                _free_list = _free_list->data.next_free;
            } else {
                if (_pages.empty() || _next_slot >= PageSize) {
                    _pages.emplace_back(std::make_unique<page>());
                    _next_slot = 0;
                }

                target = &_pages.back()->slots[_next_slot++];
            }

            assert(target != nullptr);
            assert(!target->alive);

            new (target->data.storage) T(std::forward<Args>(args)...);

            target->alive = true;
            ++_size;

            return target->value();
        }

        void erase(T* object) noexcept
        {
            if (!object) {
                return;
            }

            slot* target = slot::from_value(object);
            erase_slot(target);
        }

        template <typename Predicate>
        void erase_if(Predicate&& predicate)
        {
            for (std::size_t page_index = 0; page_index < _pages.size(); ++page_index) {
                page& current_page = *_pages[page_index];

                const std::size_t count = slots_in_page(page_index);

                for (std::size_t slot_index = 0; slot_index < count; ++slot_index) {
                    slot& current_slot = current_page.slots[slot_index];

                    if (!current_slot.alive) {
                        continue;
                    }

                    T* object = current_slot.value();

                    if (predicate(*object)) {
                        erase_slot(&current_slot);
                    }
                }
            }
        }

        template <typename Fn>
        void for_each(Fn&& fn)
        {
            for (std::size_t page_index = 0; page_index < _pages.size(); ++page_index) {
                page& current_page = *_pages[page_index];

                const std::size_t count = slots_in_page(page_index);

                for (std::size_t slot_index = 0; slot_index < count; ++slot_index) {
                    slot& current_slot = current_page.slots[slot_index];

                    if (current_slot.alive) {
                        fn(*current_slot.value());
                    }
                }
            }
        }

        template <typename Fn>
        void for_each(Fn&& fn) const
        {
            for (std::size_t page_index = 0; page_index < _pages.size(); ++page_index) {
                const page& current_page = *_pages[page_index];

                const std::size_t count = slots_in_page(page_index);

                for (std::size_t slot_index = 0; slot_index < count; ++slot_index) {
                    const slot& current_slot = current_page.slots[slot_index];

                    if (current_slot.alive) {
                        fn(*current_slot.value());
                    }
                }
            }
        }

        void clear() noexcept
        {
            for (std::size_t page_index = 0; page_index < _pages.size(); ++page_index) {
                page& current_page = *_pages[page_index];

                const std::size_t count = slots_in_page(page_index);

                for (std::size_t slot_index = 0; slot_index < count; ++slot_index) {
                    slot& current_slot = current_page.slots[slot_index];

                    if (current_slot.alive) {
                        current_slot.value()->~T();
                        current_slot.alive = false;
                    }
                }
            }

            _pages.clear();
            _free_list = nullptr;
            _next_slot = PageSize;
            _size = 0;
        }

        std::size_t size() const noexcept
        {
            return _size;
        }

        bool empty() const noexcept
        {
            return _size == 0;
        }

        std::size_t page_count() const noexcept
        {
            return _pages.size();
        }

    private:
        std::size_t slots_in_page(const std::size_t page_index) const noexcept
        {
            if (page_index + 1 < _pages.size()) {
                return PageSize;
            }

            return _next_slot;
        }

        void erase_slot(slot* target) noexcept
        {
            assert(target != nullptr);
            assert(target->alive);

            target->value()->~T();

            target->alive = false;
            target->data.next_free = _free_list;
            _free_list = target;

            --_size;
        }

    private:
        std::vector<std::unique_ptr<page>> _pages = {};
        slot* _free_list = nullptr;
        std::size_t _next_slot = PageSize;
        std::size_t _size = 0;
    };

    template <typename T, unsigned Capacity>
    class container_static_pool {
    private:
        union slot_data {
            unsigned char storage[sizeof(T)];
            unsigned next_free;
        };

        struct slot {
            slot_data data;
            bool alive;
        };

    public:
        container_static_pool()
            : _free_head(0)
            , _size(0)
        {
            for (unsigned i = 0; i < Capacity; ++i) {
                _slots[i].alive = false;
                _slots[i].data.next_free = i + 1;
            }

            _slots[Capacity - 1].data.next_free = invalid;
        }

        template <typename... Args>
        T* emplace(Args&&... args)
        {
            if (_free_head == invalid) {
                return 0;
            }

            const unsigned index = _free_head;
            slot& s = _slots[index];

            _free_head = s.data.next_free;

            new (s.data.storage) T(std::forward<Args>(args)...);

            s.alive = true;
            ++_size;

            return reinterpret_cast<T*>(s.data.storage);
        }

        void erase(T* object)
        {
            if (!object) {
                return;
            }

            const unsigned index = index_from_object(object);
            slot& s = _slots[index];

            reinterpret_cast<T*>(s.data.storage)->~T();

            s.alive = false;
            s.data.next_free = _free_head;
            _free_head = index;

            --_size;
        }

        template <typename Predicate>
        void erase_if(Predicate pred)
        {
            for (unsigned i = 0; i < Capacity; ++i) {
                slot& s = _slots[i];

                if (!s.alive) {
                    continue;
                }

                T* object = reinterpret_cast<T*>(s.data.storage);

                if (pred(*object)) {
                    object->~T();

                    s.alive = false;
                    s.data.next_free = _free_head;
                    _free_head = i;

                    --_size;
                }
            }
        }

        template <typename Fn>
        void for_each(Fn fn)
        {
            for (unsigned i = 0; i < Capacity; ++i) {
                slot& s = _slots[i];

                if (s.alive) {
                    fn(*reinterpret_cast<T*>(s.data.storage));
                }
            }
        }

        unsigned size() const
        {
            return _size;
        }

    private:
        unsigned index_from_object(T* object) const
        {
            const unsigned char* ptr = reinterpret_cast<const unsigned char*>(object);
            const unsigned char* base = reinterpret_cast<const unsigned char*>(&_slots[0]);

            return static_cast<unsigned>((ptr - base) / sizeof(slot));
        }

    private:
        static const unsigned invalid = ~0u;

        slot _slots[Capacity];
        unsigned _free_head;
        unsigned _size;
    };

}
}
