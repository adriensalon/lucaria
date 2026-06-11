#pragma once

#include <lucaria/bin/types_math.hpp>
#include <lucaria/core/app_error.hpp>
#include <lucaria/core/assets_async.hpp>
#include <lucaria/core/assets_storage.hpp>
#include <lucaria/core/reload_filewatch.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace lucaria {
namespace detail {

    struct manager_assets;
    struct manager_window;

    struct flag_refcount_control {
        std::atomic<uint32> count = { 0 };
    };

    struct flag_refcount {
        flag_refcount() noexcept = default;
        flag_refcount(const flag_refcount& other) noexcept;
        flag_refcount& operator=(const flag_refcount& other) noexcept;
        flag_refcount(flag_refcount&& other) noexcept;
        flag_refcount& operator=(flag_refcount&& other) noexcept;
        ~flag_refcount();

        explicit flag_refcount(flag_refcount_control* control) noexcept;
        void reset() noexcept;
        bool owns() const noexcept;
        bool is_last_owner() const noexcept;
        uint32 use_count() const noexcept;

    private:
        void _retain() noexcept;
        void _release() noexcept;

    private:
        flag_refcount_control* _control = nullptr;
    };

    template <typename Asset>
    struct assets_cell {
        assets_async_slot<Asset> fetched = {};
        std::string cache_id = {};
        uint32 current_version = 0u;
        flag_refcount_control refcount_control = {};
        std::vector<object_filewatched_path> object_filewatched_paths = {};
        std::function<void()> refetch = nullptr;
    };

    struct assets_filewatch_change {
        std::type_index asset_type;
        void* cell = nullptr;
        std::string cache_id = {};
        std::filesystem::path path = {};
    };

    struct assets_buffer_base {
        virtual ~assets_buffer_base() = default;
        virtual void clear() = 0;
        virtual void gc_unused() = 0;
        virtual void poll_filewatch_changes(std::vector<assets_filewatch_change>& changes) = 0;
        virtual void refetch_filewatch_changes(std::vector<assets_filewatch_change>& changes) = 0;
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

        void reset_cell_fetch(assets_cell<Asset>* cell, assets_async_slot<Asset>&& value)
        {
            LUCARIA_DEBUG_ASSERT(cell, "Asset cell was nullptr");
            cell->fetched = {};
            cell->fetched = std::move(value);
            cell->object_filewatched_paths.clear();
            cell->fetched.on_ready([cell]() {
                cell->current_version++;
            });
        }

        void set_cell(
            assets_cell<Asset>* cell,
            assets_async_slot<Asset>&& value,
            std::string cache_id = {})
        {
            LUCARIA_DEBUG_ASSERT(cell, "Asset cell was nullptr");
            _erase_id(cell);
            cell->cache_id = std::move(cache_id);
            reset_cell_fetch(cell, std::move(value));
			if (cell->cache_id.empty()) {
                return;
            }
            _cells_by_id.emplace(cell->cache_id, cell);
        }

        [[nodiscard]] assets_cell<Asset>* create_cell(assets_async_slot<Asset>&& value, std::string cache_id = {})
        {
            assets_cell<Asset>* _cell = create_cell();
            set_cell(_cell, std::move(value), std::move(cache_id));
            return _cell;
        }

        [[nodiscard]] assets_cell<Asset>* find_by_id(const std::string& cache_id) const
        {
            if (cache_id.empty()) {
                return nullptr;
            }
            auto _iterator = _cells_by_id.find(cache_id);
            return _iterator == _cells_by_id.end() ? nullptr : _iterator->second;
        }

        [[nodiscard]] assets_cell<Asset>* get_or_create_by_id(std::string cache_id, std::function<assets_async_slot<Asset>()> create_fetch)
        {
            if (assets_cell<Asset>* _existing = find_by_id(cache_id)) {
                return _existing;
            }
            return create_cell(create_fetch(), std::move(cache_id));
        }

        void destroy_cell(assets_cell<Asset>* cell)
        {
            if (!cell) {
                return;
            }
            _erase_id(cell);
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
                _erase_id(&cell);
                return true;
            });
        }

        void poll_filewatch_changes(std::vector<assets_filewatch_change>& changes) override
        {
            collect_filewatch_changes(changes, false);
        }

        void refetch_filewatch_changes(std::vector<assets_filewatch_change>& changes) override
        {
            collect_filewatch_changes(changes, true);
        }

    private:
        void collect_filewatch_changes(std::vector<assets_filewatch_change>& changes, bool refetch_changed_cells)
        {
            _cells.for_each([&](assets_cell<Asset>& cell) {
                bool _should_refetch_cell = false;
                for (object_filewatched_path& _watched_path : cell.object_filewatched_paths) {
                    if (_watched_path.has_changed()) {
                        _should_refetch_cell = true;
                        changes.push_back({ std::type_index(typeid(Asset)), &cell, cell.cache_id, _watched_path.get() });
                    }
                }
                if (refetch_changed_cells && _should_refetch_cell && cell.refetch) {
                    cell.refetch();
                }
            });
        }

    private:
        assets_paged_buffer<assets_cell<Asset>> _cells = {};
        std::unordered_map<std::string, assets_cell<Asset>*> _cells_by_id = {};

        void _erase_id(assets_cell<Asset>* cell)
        {
            if (cell->cache_id.empty()) {
                return;
            }

            auto it = _cells_by_id.find(cell->cache_id);
            if (it != _cells_by_id.end() && it->second == cell) {
                _cells_by_id.erase(it);
            }
        }
    };

    struct assets_registry {
        assets_registry() = default;
        assets_registry(const assets_registry&) = delete;
        assets_registry& operator=(const assets_registry&) = delete;
        assets_registry(assets_registry&&) = delete;
        assets_registry& operator=(assets_registry&&) = delete;

        template <typename Asset>
        [[nodiscard]] assets_buffer<Asset>& get()
        {
            const std::type_index type = std::type_index(typeid(Asset));
            auto iterator = _buffers.find(type);
            if (iterator == _buffers.end()) {
                std::unique_ptr<assets_buffer<Asset>> buffer = std::make_unique<assets_buffer<Asset>>();
                assets_buffer<Asset>* raw = buffer.get();
                _buffers.emplace(type, std::move(buffer));
                return *raw;
            }
            return *static_cast<assets_buffer<Asset>*>(iterator->second.get());
        }

        template <typename Asset>
        [[nodiscard]] const assets_buffer<Asset>& get() const
        {
            return const_cast<assets_registry*>(this)->get<Asset>();
        }

        void clear()
        {
            for (auto& [type, buffer] : _buffers) {
                buffer->clear();
            }
        }

        void clear_buffers_for_reload()
        {
            _buffers.clear();
        }

        void gc_unused()
        {
            for (auto& [type, buffer] : _buffers) {
                buffer->gc_unused();
            }
        }

        [[nodiscard]] std::vector<assets_filewatch_change> poll_filewatch_changes()
        {
            std::vector<assets_filewatch_change> changes;
            for (auto& [type, buffer] : _buffers) {
                buffer->poll_filewatch_changes(changes);
            }
            return changes;
        }

        [[nodiscard]] std::vector<assets_filewatch_change> refetch_filewatch_changes()
        {
            std::vector<assets_filewatch_change> changes;
            for (auto& [type, buffer] : _buffers) {
                buffer->refetch_filewatch_changes(changes);
            }
            return changes;
        }

        [[nodiscard]] bool has_filewatch_changes()
        {
            return !poll_filewatch_changes().empty();
        }

        template <typename Asset, typename Configure>
        [[nodiscard]] assets_cell<Asset>& fetch(
            manager_assets& objects,
            std::string cache_id,
            Configure&& configure,
            manager_window* window = nullptr);

        template <typename Asset, typename... Args>
        [[nodiscard]] assets_cell<Asset>& create(Args&&... args)
        {
            return *get<Asset>().create_cell(assets_async_slot<Asset>(Asset(std::forward<Args>(args)...)));
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<assets_buffer_base>> _buffers = {};
    };

}
}
