#pragma once

#include <filesystem>
#include <vector>

namespace lucaria {
namespace detail {

    template <typename T, std::uint32_t PageSize = 256>
    struct generics_paged_buffer {
        generics_paged_buffer() = default;
        generics_paged_buffer(const generics_paged_buffer&) = delete;
        generics_paged_buffer& operator=(const generics_paged_buffer&) = delete;
        generics_paged_buffer(generics_paged_buffer&&) = delete;
        generics_paged_buffer& operator=(generics_paged_buffer&&) = delete;

        ~generics_paged_buffer()
        {
            clear();
        }

        struct slot {
            alignas(T) std::byte raw[sizeof(T)];
            bool alive = false;

            T* value() noexcept
            {
                return std::launder(reinterpret_cast<T*>(raw));
            }

            const T* value() const noexcept
            {
                return std::launder(reinterpret_cast<const T*>(raw));
            }
        };

        struct page {
            slot slots[PageSize];
            std::uint32_t alive_count = 0;
        };

        template <typename... Args>
        T* create(Args&&... args)
        {
            auto [pg, index] = assure_free_slot();

            slot& s = pg->slots[index];
            std::construct_at(s.value(), std::forward<Args>(args)...);
            s.alive = true;
            ++pg->alive_count;

            return s.value();
        }

        void destroy(T* ptr)
        {
            if (!ptr) {
                return;
            }

            for (page* pg : _pages) {
                auto* begin = pg->slots;
                auto* end = pg->slots + PageSize;

                for (slot* s = begin; s != end; ++s) {
                    if (s->alive && s->value() == ptr) {
                        std::destroy_at(s->value());
                        s->alive = false;
                        --pg->alive_count;
                        _free.push_back({ pg, static_cast<std::uint32_t>(s - begin) });
                        return;
                    }
                }
            }
        }

        template <typename Callback>
        void for_each(Callback&& callback)
        {
            for (page* pg : _pages) {
                for (slot& s : pg->slots) {
                    if (s.alive) {
                        callback(*s.value());
                    }
                }
            }
        }

        template <typename Callback>
        void erase_if(Callback&& callback)
        {
            for (page* pg : _pages) {
                for (std::uint32_t i = 0; i < PageSize; ++i) {
                    slot& s = pg->slots[i];

                    if (!s.alive) {
                        continue;
                    }

                    T* value = s.value();

                    if (callback(*value)) {
                        std::destroy_at(value);
                        s.alive = false;
                        --pg->alive_count;
                        _free.push_back({ pg, i });
                    }
                }
            }
        }

        void clear()
        {
            for (page* pg : _pages) {
                for (slot& s : pg->slots) {
                    if (s.alive) {
                        std::destroy_at(s.value());
                        s.alive = false;
                    }
                }

                delete pg;
            }

            _pages.clear();
            _free.clear();
        }

    private:
        struct free_slot {
            page* pg = nullptr;
            std::uint32_t index = 0;
        };

        std::vector<page*> _pages = {};
        std::vector<free_slot> _free = {};

        std::pair<page*, std::uint32_t> assure_free_slot()
        {
            if (!_free.empty()) {
                free_slot f = _free.back();
                _free.pop_back();
                return { f.pg, f.index };
            }

            page* pg = new page {};
            _pages.push_back(pg);

            for (std::uint32_t i = PageSize; i-- > 1;) {
                _free.push_back({ pg, i });
            }

            return { pg, 0 };
        }
    };

}
}
