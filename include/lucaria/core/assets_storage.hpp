#pragma once

#include <lucaria/bin/types_containers.hpp>
#include <lucaria/bin/types_math.hpp>

namespace lucaria {
namespace detail {

    template <typename AssetCell, uint32 PageSize = 256u>
    struct assets_paged_buffer {
        assets_paged_buffer() = default;
        assets_paged_buffer(const assets_paged_buffer&) = delete;
        assets_paged_buffer& operator=(const assets_paged_buffer&) = delete;
        assets_paged_buffer(assets_paged_buffer&&) = delete;
        assets_paged_buffer& operator=(assets_paged_buffer&&) = delete;

        ~assets_paged_buffer()
        {
            clear();
        }

        struct slot {
            alignas(AssetCell) std::byte raw[sizeof(AssetCell)];
            bool is_alive = false;

            AssetCell* value() noexcept
            {
                return std::launder(reinterpret_cast<AssetCell*>(raw));
            }

            const AssetCell* value() const noexcept
            {
                return std::launder(reinterpret_cast<const AssetCell*>(raw));
            }
        };

        struct page {
            slot slots[PageSize];
            uint32 alive_count = 0;
        };

        template <typename... Args>
        AssetCell* create(Args&&... args)
        {
            auto [_page, _index] = assure_free_slot();
            slot& _slot = _page->slots[_index];
            std::construct_at(_slot.value(), std::forward<Args>(args)...);
            _slot.is_alive = true;
            ++_page->alive_count;
            return _slot.value();
        }

        void destroy(AssetCell* ptr)
        {
            if (!ptr) {
                return;
            }
            for (page* _page : _pages) {
                auto* _begin = _page->slots;
                auto* _end = _page->slots + PageSize;
                for (slot* _slot = _begin; _slot != _end; ++_slot) {
                    if (_slot->is_alive && _slot->value() == ptr) {
                        std::destroy_at(_slot->value());
                        _slot->is_alive = false;
                        --_page->alive_count;
                        _free.push_back({ _page, static_cast<uint32>(_slot - _begin) });
                        return;
                    }
                }
            }
        }

        template <typename Callback>
        void for_each(Callback&& callback)
        {
            for (page* _page : _pages) {
                for (slot& _slot : _page->slots) {
                    if (_slot.is_alive) {
                        callback(*_slot.value());
                    }
                }
            }
        }

        template <typename Callback>
        void erase_if(Callback&& callback)
        {
            for (page* _page : _pages) {
                for (uint32 _index = 0; _index < PageSize; ++_index) {
                    slot& _slot = _page->slots[_index];
                    if (!_slot.is_alive) {
                        continue;
                    }
                    AssetCell* _value = _slot.value();
                    if (callback(*_value)) {
                        std::destroy_at(_value);
                        _slot.is_alive = false;
                        --_page->alive_count;
                        _free.push_back({ _page, _index });
                    }
                }
            }
        }

        void clear()
        {
            for (page* _page : _pages) {
                for (slot& s : _page->slots) {
                    if (s.is_alive) {
                        std::destroy_at(s.value());
                        s.is_alive = false;
                    }
                }
                delete _page;
            }
            _pages.clear();
            _free.clear();
        }

    private:
        struct free_slot {
            page* slot_page = nullptr;
            uint32 index = 0;
        };

        std::vector<page*> _pages = {};
        std::vector<free_slot> _free = {};

        std::pair<page*, uint32> assure_free_slot()
        {
            if (!_free.empty()) {
                free_slot _free_slot = _free.back();
                _free.pop_back();
                return { _free_slot.slot_page, _free_slot.index };
            }
            page* _page = new page();
            _pages.push_back(_page);
            for (uint32 _page_index = PageSize; _page_index-- > 1;) {
                _free.push_back({ _page, _page_index });
            }
            return { _page, 0 };
        }
    };

}
}
