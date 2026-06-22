#pragma once

#include <lucaria/engine/context_serialize.hpp>

namespace lucaria {
namespace detail {

    template <typename ValueType, typename Callback>
    struct load_context_value_task final : load_context_task_base {
        assets_async_slot<ValueType> value = {};
        Callback callback;
        bool done = false;

        load_context_value_task(std::future<ValueType>&& future, Callback&& callback)
            : value(std::move(future), [](const ValueType& value) {
                return value;
            })
            , callback(std::move(callback))
        {
        }

        bool poll() override
        {
            if (done) {
                return true;
            }
            if (!value.has_value()) {
                return false;
            }
            callback(std::as_const(value.value()));
            done = true;
            return true;
        }

        bool finished() const override
        {
            return done;
        }
    };

    template <typename AssetType>
    void track_asset_cell_fetch_path(assets_cell<AssetType>* cell, const std::filesystem::path& path)
    {
        if (cell == nullptr) {
            return;
        }

        std::vector<object_filewatched_path>& watched_paths = cell->object_filewatched_paths;
        const auto existing = std::find_if(watched_paths.begin(), watched_paths.end(), [&](const object_filewatched_path& watched_path) {
            return watched_path.get() == path;
        });
        if (existing == watched_paths.end()) {
            watched_paths.emplace_back(path);
        }
    }

    template <typename AssetType>
    void mark_asset_cell_ready(assets_cell<AssetType>* cell)
    {
        if (cell != nullptr) {
            cell->fetched.mark_ready();
        }
    }

    template <typename AssetType, typename ArchiveType>
    struct load_storage_context final : context_load_storage {
        assets_cell<AssetType>* cell;

        load_storage_context(ArchiveType& archive, manager_assets& objects, assets_cell<AssetType>* cell, AssetType&, manager_window* window = nullptr)
            : context_load_storage(archive, objects, window)
            , cell(cell)
        {
        }

    protected:
        void track_resolved_fetch_path(const std::filesystem::path& path) override
        {
            track_asset_cell_fetch_path(cell, path);
        }

        void notify_finished() override
        {
            mark_asset_cell_ready(cell);
        }
    };

    template <typename AssetType>
    struct runtime_storage_context final : context_load_storage {
        assets_cell<AssetType>* cell;

        runtime_storage_context(manager_assets& objects, assets_cell<AssetType>* cell, AssetType&, manager_window* runtime_window = nullptr)
            : context_load_storage(objects, runtime_window)
            , cell(cell)
        {
        }

    protected:
        void track_resolved_fetch_path(const std::filesystem::path& path) override
        {
            track_asset_cell_fetch_path(cell, path);
        }

        void notify_finished() override
        {
            mark_asset_cell_ready(cell);
        }
    };

}
}
