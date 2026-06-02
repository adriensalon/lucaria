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
        std::optional<std::filesystem::path> origin_path = {};
        uint32 current_version = 0u;
        flag_refcount_control refs = {};
    };

    struct assets_buffer_base {
        virtual ~assets_buffer_base() = default;
        virtual void clear() = 0;
        virtual void gc_unused() = 0;
    };

    template <typename Asset>
    struct assets_buffer final : assets_buffer_base {
        using cell_type = assets_cell<Asset>;

        template <typename Callback>
        void for_each_cell(Callback&& callback) const
        {
            const_cast<assets_buffer*>(this)->_cells.for_each(
                [&](cell_type& cell) {
                    callback(cell);
                });
        }

        [[nodiscard]] cell_type* create_cell()
        {
            return _cells.create();
        }

        void set_cell(
            cell_type* cell,
            container_async<Asset>&& value,
            std::optional<std::filesystem::path> path = std::nullopt)
        {
            LUCARIA_DEBUG_ASSERT(cell, "Asset cell was nullptr");

            if (cell->origin_path) {
                auto it = _cells_by_path.find(*cell->origin_path);
                if (it != _cells_by_path.end() && it->second == cell) {
                    _cells_by_path.erase(it);
                }
            }

            cell->fetched = std::move(value);
            cell->origin_path = std::move(path);

            cell->fetched.on_ready([cell]() {
                cell->current_version++;
            });

            if (cell->origin_path) {
                _cells_by_path.emplace(*cell->origin_path, cell);
            }
        }

        [[nodiscard]] cell_type* create_cell(
            container_async<Asset>&& value,
            std::optional<std::filesystem::path> path = std::nullopt)
        {
            cell_type* cell = create_cell();
            set_cell(cell, std::move(value), std::move(path));
            return cell;
        }

        [[nodiscard]] cell_type* find_by_path(const std::filesystem::path& path) const
        {
            auto it = _cells_by_path.find(path);
            return it == _cells_by_path.end() ? nullptr : it->second;
        }

        [[nodiscard]] cell_type* get_or_create_by_path(
            const std::filesystem::path& path,
            std::function<container_async<Asset>()> create_fetch)
        {
            if (cell_type* existing = find_by_path(path)) {
                return existing;
            }

            return create_cell(create_fetch(), path);
        }

        void destroy_cell(cell_type* cell)
        {
            if (!cell) {
                return;
            }

            erase_path(cell);
            _cells.destroy(cell);
        }

        void clear() override
        {
            _cells_by_path.clear();
            _cells.clear();
        }

        void gc_unused() override
        {
            _cells.erase_if([this](cell_type& cell) {
                if (!cell.fetched.has_value()) {
                    return false;
                }

                if (cell.refs.count.load(std::memory_order_acquire) != 0) {
                    return false;
                }

                erase_path(&cell);
                return true;
            });
        }

    private:
        generics_paged_buffer<cell_type> _cells = {};
        std::unordered_map<std::filesystem::path, cell_type*> _cells_by_path = {};

        void erase_path(cell_type* cell)
        {
            if (!cell->origin_path) {
                return;
            }

            auto it = _cells_by_path.find(*cell->origin_path);
            if (it != _cells_by_path.end() && it->second == cell) {
                _cells_by_path.erase(it);
            }
        }
    };

}
}
