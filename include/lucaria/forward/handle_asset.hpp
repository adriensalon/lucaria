#pragma once

#include <lucaria/core/assets_buffer.hpp>

namespace lucaria {

template <typename AssetType>
struct handle_asset {

    bool has_value() const
    {
        return _cached != nullptr && _cached->fetched.has_value();
    }

    explicit operator bool() const
    {
        return has_value();
    }

    AssetType& value()
    {
        LUCARIA_DEBUG_ASSERT(_cached != nullptr, "Invalid asset handle");
        return _cached->fetched.value();
    }

    const AssetType& value() const
    {
        LUCARIA_DEBUG_ASSERT(_cached != nullptr, "Invalid asset handle");
        return _cached->fetched.value();
    }

    AssetType* operator->()
    {
        return &value();
    }

    const AssetType* operator->() const
    {
        return &value();
    }

    AssetType& operator*()
    {
        return value();
    }

    const AssetType& operator*() const
    {
        return value();
    }

    void reset()
    {
        _cached = nullptr;
        _refcount = {};
    }

    bool empty() const
    {
        return _cached == nullptr;
    }

// private for API purposes, public for existing engine internals during migration.
    detail::flag_refcount _refcount = {};
    detail::assets_cell<AssetType>* _cached = nullptr;

    template <typename ArchiveType>
    void save(ArchiveType& archive) const;

    template <typename ArchiveType>
    void load(ArchiveType& archive);

    friend struct context_object;
    friend class cereal::access;
};

}
