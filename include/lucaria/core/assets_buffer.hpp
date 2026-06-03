#pragma once

#include <lucaria/core/generics_memory.hpp>
#include <lucaria/core/utils_async.hpp>
#include <lucaria/core/utils_error.hpp>
#include <lucaria/core/utils_refcount.hpp>

namespace lucaria {
namespace detail {

    template <typename Asset>
    struct assets_cell {
        container_async<Asset> fetched = {};
        std::string cache_id = {};
        uint32 current_version = 0u;
        flag_refcount_control refcount_control = {};
    };

    struct assets_buffer_base {
        virtual ~assets_buffer_base() = default;
        virtual void clear() = 0;
        virtual void gc_unused() = 0;
    };

    template <typename Asset>
    struct assets_buffer final : assets_buffer_base {

        template <typename Callback>
        void for_each_cell(Callback&& callback) const
        {
            const_cast<assets_buffer*>(this)->_cells.for_each(
                [&](assets_cell<Asset>& cell) {
                    callback(cell);
                });
        }

        [[nodiscard]] assets_cell<Asset>* create_cell()
        {
            return _cells.create();
        }

        void set_cell(
            assets_cell<Asset>* cell,
            container_async<Asset>&& value,
            std::string cache_id = {})
        {
            LUCARIA_DEBUG_ASSERT(cell, "Asset cell was nullptr");

            erase_id(cell);

            cell->fetched = std::move(value);
            cell->cache_id = std::move(cache_id);

            cell->fetched.on_ready([cell]() {
                cell->current_version++;
            });

            insert_id(cell);
        }

        [[nodiscard]] assets_cell<Asset>* create_cell(
            container_async<Asset>&& value,
            std::string cache_id = {})
        {
            assets_cell<Asset>* cell = create_cell();
            set_cell(cell, std::move(value), std::move(cache_id));
            return cell;
        }

        [[nodiscard]] assets_cell<Asset>* find_by_id(const std::string& cache_id) const
        {
            if (cache_id.empty()) {
                return nullptr;
            }

            auto it = _cells_by_id.find(cache_id);
            return it == _cells_by_id.end() ? nullptr : it->second;
        }

        [[nodiscard]] assets_cell<Asset>* get_or_create_by_id(
            std::string cache_id,
            std::function<container_async<Asset>()> create_fetch)
        {
            if (assets_cell<Asset>* existing = find_by_id(cache_id)) {
                return existing;
            }

            return create_cell(create_fetch(), std::move(cache_id));
        }

        void destroy_cell(assets_cell<Asset>* cell)
        {
            if (!cell) {
                return;
            }

            erase_id(cell);
            _cells.destroy(cell);
        }

        void clear() override
        {
            _cells_by_id.clear();
            _cells.clear();
        }

        void gc_unused() override
        {
            _cells.erase_if([this](assets_cell<Asset>& cell) {
                if (!cell.fetched.has_value()) {
                    return false;
                }

                if (cell.refcount_control.count.load(std::memory_order_acquire) != 0) {
                    return false;
                }

                erase_id(&cell);
                return true;
            });
        }

    private:
        generics_paged_buffer<assets_cell<Asset>> _cells = {};
        std::unordered_map<std::string, assets_cell<Asset>*> _cells_by_id = {};

        void erase_id(assets_cell<Asset>* cell)
        {
            if (cell->cache_id.empty()) {
                return;
            }

            auto it = _cells_by_id.find(cell->cache_id);
            if (it != _cells_by_id.end() && it->second == cell) {
                _cells_by_id.erase(it);
            }
        }

        void insert_id(assets_cell<Asset>* cell)
        {
            if (cell->cache_id.empty()) {
                return;
            }
            _cells_by_id.emplace(cell->cache_id, cell);
        }
    };

}
}
