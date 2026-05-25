#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <lucaria/bin/types_math.hpp>
#include <lucaria/core/utils_async.hpp>
#include <lucaria/core/utils_refcount.hpp>

namespace lucaria {
namespace detail {

    template <typename ObjectType>
    struct container_cache {
        container_async<ObjectType> fetched = {};
        std::optional<std::filesystem::path> origin_path = {};
        std::atomic<uint32> current_version = { 0 };
        flag_refcount_control refs = {};
    };

    template <typename ObjectType>
    struct container_cache_vector {
        std::vector<std::unique_ptr<container_cache<ObjectType>>> cells = {};
        std::unordered_map<std::filesystem::path, container_cache<ObjectType>*> cells_by_path = {};
        std::mutex set_mutex = {};

        void set_cell(container_cache<ObjectType>* cell, container_async<ObjectType>&& value, std::optional<std::filesystem::path> path = std::nullopt)
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

        container_cache<ObjectType>* create_cell()
        {
            std::unique_ptr<container_cache<ObjectType>> _cell = std::make_unique<container_cache<ObjectType>>();
            container_cache<ObjectType>* _raw_cell = _cell.get();
            cells.emplace_back(std::move(_cell));
            return _raw_cell;
        }

        container_cache<ObjectType>* create_cell(container_async<ObjectType>&& value, std::optional<std::filesystem::path> path = std::nullopt)
        {
            container_cache<ObjectType>* _cell = create_cell();
            set_cell(_cell, std::move(value), std::move(path));
            return _cell;
        }

        [[nodiscard]] container_cache<ObjectType>* find_by_path(const std::filesystem::path& path) const
        {
            typename std::unordered_map<std::filesystem::path, container_cache<ObjectType>*>::const_iterator _iterator = cells_by_path.find(path);
            return _iterator == cells_by_path.end() ? nullptr : _iterator->second;
        }

        [[nodiscard]] container_cache<ObjectType>* get_or_create_by_path(const std::filesystem::path& path, std::function<container_async<ObjectType>()> create_fetch)
        {
            if (container_cache<ObjectType>* _existing = find_by_path(path)) {
                return _existing;
            }

            return create_cell(create_fetch(), path);
        }

        void destroy_cell(container_cache<ObjectType>* cell)
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

        void gc_unused()
        {
            std::lock_guard _lock(set_mutex);
            std::erase_if(cells, [this](const std::unique_ptr<container_cache<ObjectType>>& _cached_ptr) {
                container_cache<ObjectType>* _cached = _cached_ptr.get();
                if (!_cached->fetched.has_value()) {
                    return false;
                }
                if (_cached->refs.count.load(std::memory_order_acquire) != 0) {
                    return false;
                }

                if (_cached->origin_path) {
                    auto it = cells_by_path.find(*_cached->origin_path);
                    if (it != cells_by_path.end() && it->second == _cached) {
                        cells_by_path.erase(it);
                    }
                }
                return true;
            });
        }
    };

    // mappings

    template <typename ObjectType>
    struct mappings_container_cache_vector {

        std::unordered_map<const container_cache<ObjectType>*, uint32> ids = {};
        uint32 next_id = 1;

        [[nodiscard]] uint32 get(const container_cache<ObjectType>* resource) const
        {
            if (!resource) {
                LUCARIA_DEBUG_ERROR("Object implementation was nullptr");
                return 0;
            }

            if (typename std::unordered_map<const container_cache<ObjectType>*, uint32>::const_iterator it = ids.find(resource); it != ids.end()) {
                return it->second;
            }

            LUCARIA_DEBUG_ERROR("Object was not registered before component recipe save");
            return 0;
        }

        [[nodiscard]] uint32 get_or_create(const container_cache<ObjectType>* resource)
        {
            if (!resource) {
                LUCARIA_DEBUG_ERROR("Object implementation was nullptr");
                return 0;
            }

            if (typename std::unordered_map<const container_cache<ObjectType>*, uint32>::const_iterator it = ids.find(resource); it != ids.end()) {
                return it->second;
            }

            const uint32 id = next_id++;
            ids.emplace(resource, id);
            return id;
        }
    };
}
}