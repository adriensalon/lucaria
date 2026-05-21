#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <variant>

#include <lucaria/core/fetch.hpp>

namespace lucaria {
namespace detail {

    template <typename ImplementationType>
    struct implementation_container {
        async_container<ImplementationType> fetched = {};
        std::optional<std::filesystem::path> origin_path = {};
        std::atomic<std::uint32_t> current_version = { 0 };
    };

    template <typename ImplementationType>
    struct implementation_manager {
        std::vector<std::unique_ptr<implementation_container<ImplementationType>>> cells = {};
        std::unordered_map<std::filesystem::path, implementation_container<ImplementationType>*> cells_by_path = {};
        std::mutex set_mutex = {};

        void set_cell(implementation_container<ImplementationType>* cell, async_container<ImplementationType>&& value, std::optional<std::filesystem::path> path = std::nullopt)
        {
            std::lock_guard _lock(set_mutex);

            if (cell->origin_path) {
                cells_by_path.erase(*cell->origin_path);
            }
            cell->fetched = std::move(value);
            cell->origin_path = std::move(path);
            cell->fetched.on_ready([cell]() {
                cell->current_version.fetch_add(1, std::memory_order_release);
            });

            if (cell->origin_path) {
                cells_by_path.emplace(*cell->origin_path, cell);
            }
        }

        implementation_container<ImplementationType>* create_cell()
        {
            std::unique_ptr<implementation_container<ImplementationType>> _cell = std::make_unique<implementation_container<ImplementationType>>();
            implementation_container<ImplementationType>* _raw_cell = _cell.get();
            cells.emplace_back(std::move(_cell));
            return _raw_cell;
        }

        implementation_container<ImplementationType>* create_cell(async_container<ImplementationType>&& value, std::optional<std::filesystem::path> path = std::nullopt)
        {
            implementation_container<ImplementationType>* _cell = create_cell();
            set_cell(_cell, std::move(value), std::move(path));
            return _cell;
        }

        [[nodiscard]] implementation_container<ImplementationType>* find_by_path(const std::filesystem::path& path) const
        {
            typename std::unordered_map<std::filesystem::path, implementation_container<ImplementationType>*>::const_iterator _iterator = cells_by_path.find(path);
            return _iterator == cells_by_path.end() ? nullptr : _iterator->second;
        }

        [[nodiscard]] implementation_container<ImplementationType>* get_or_create_by_path(const std::filesystem::path& path, std::function<async_container<ImplementationType>()> create_fetch)
        {
            if (implementation_container<ImplementationType>* _existing = find_by_path(path)) {
                return _existing;
            }

            return create_cell(create_fetch(), path);
        }

        void destroy_cell(implementation_container<ImplementationType>* cell)
        {
            if (!cell) {
                return;
            }

            if (cell->origin_path) {
                cells_by_path.erase(*cell->origin_path);
            }

            std::erase_if(cells, [cell](const auto& ptr) {
                return ptr.get() == cell;
            });
        }

        void clear()
        {
            cells_by_path.clear();
            cells.clear();
        }
    };
}
}