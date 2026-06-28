#pragma once

#include <string>
#include <vector>

#include <lucaria/core/manager_assets.hpp>
#include <lucaria/core/serialize_containers.hpp>
#include <lucaria/core/serialize_mappings.hpp>
#include <lucaria/core/user_assets.hpp>

namespace lucaria {
namespace detail {

    struct storage_user_asset_group {
        std::string type_id = {};
        manager_assets* objects = nullptr;
        const user_asset_type_callbacks* callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));
            save_registered_object(archive, *callbacks, *objects);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            archive(cereal::make_nvp("type_id", type_id));
            mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            LUCARIA_DEBUG_ASSERT(_mappings.loading_objects != nullptr, "Missing manager_object while loading user asset group");
            auto _iterator = _mappings.loading_objects->user_asset_types.find(type_id);
            LUCARIA_DEBUG_ASSERT(_iterator != _mappings.loading_objects->user_asset_types.end(), "Unknown user asset type while loading snapshot");
            load_registered_object(archive, _iterator->second, *_mappings.loading_objects, "user_asset_group", type_id);
        }
    };

    template <typename Asset>
    struct direct_asset_cell_save {
        uint32 save_id = 0;
        Asset* value = nullptr;

        void save(context_save_storage& context) const
        {
            context.field("object_save_id", save_id);
            LUCARIA_DEBUG_ASSERT(value != nullptr, "Missing asset value while saving asset cell");
            save_user_asset_value(*value, context);
        }
    };

    template <typename Asset>
    struct direct_asset_cell_load {
        uint32 save_id = 0;
        assets_buffer<Asset>* buffer = nullptr;
        mappings_container_cache_vector_load<Asset>* ids = nullptr;

        void load(context_load_storage& context)
        {
            context.field("object_save_id", save_id);
            LUCARIA_DEBUG_ASSERT(buffer != nullptr && ids != nullptr, "Missing asset buffer or mapping while loading asset cell");
            assets_cell<Asset>* _cell = buffer->create_cell(assets_async_slot<Asset>::pending(Asset {}));
            Asset& _asset = _cell->fetched.emplaced_value();
            const bool _is_loaded = visit_load_archive(context.archive, [&](auto& archive_value) {
                using child_archive_type = std::decay_t<decltype(archive_value)>;
                std::shared_ptr<load_storage_context<Asset, child_archive_type>> child_context = std::make_shared<load_storage_context<Asset, child_archive_type>>(archive_value, context.objects, _cell, _asset, context.window());
                context.objects.active_load_contexts.emplace_back(child_context);
                load_user_asset_value(_asset, *child_context);
                child_context->close();
            });
            LUCARIA_DEBUG_ASSERT(_is_loaded, "Cannot load asset cell without an archive-backed context_load_storage");
            ids->set(save_id, _cell);
        }
    };

    template <typename Asset>
    struct direct_asset_buffer_save {
        const assets_buffer<Asset>* buffer = nullptr;
        mappings_container_cache_vector_save<Asset>* ids = nullptr;

        void save(context_save_storage& context) const
        {
            LUCARIA_DEBUG_ASSERT(buffer != nullptr && ids != nullptr, "Missing asset buffer or mapping while saving asset buffer");
            std::size_t _count = 0;
            buffer->for_each_cell([&](assets_cell<Asset>& cell) {
                if (cell.fetched.has_value()) {
                    ++_count;
                }
            });
            context.field("count", _count);
            std::size_t _index = 0;
            buffer->for_each_cell([&](assets_cell<Asset>& cell) {
                if (!cell.fetched.has_value()) {
                    return;
                }
                direct_asset_cell_save<Asset> _asset { ids->get_or_create(&cell), &cell.fetched.value() };
                context.field("asset_" + std::to_string(_index), _asset);
                ++_index;
            });
        }
    };

    template <typename Asset>
    struct direct_asset_buffer_load {
        assets_buffer<Asset>* buffer = nullptr;
        mappings_container_cache_vector_load<Asset>* ids = nullptr;

        void load(context_load_storage& context)
        {
            LUCARIA_DEBUG_ASSERT(buffer != nullptr && ids != nullptr, "Missing asset buffer or mapping while loading asset buffer");
            std::size_t _count = 0;
            context.field("count", _count);
            for (std::size_t _index = 0; _index < _count; ++_index) {
                direct_asset_cell_load<Asset> asset { 0, buffer, ids };
                context.field("asset_" + std::to_string(_index), asset);
            }
        }
    };

    template <typename Asset>
    void save_asset_buffer_direct(context_save_storage& context, std::string_view name, manager_assets& objects, mappings_container_cache_vector_save<Asset>& ids)
    {
        direct_asset_buffer_save<Asset> _buffer { &objects.template get_asset_buffer<Asset>(), &ids };
        context.field(name, _buffer);
    }

    template <typename Asset>
    void load_asset_buffer_direct(context_load_storage& context, std::string_view name, manager_assets& objects, mappings_container_cache_vector_load<Asset>& ids)
    {
        direct_asset_buffer_load<Asset> _buffer { &objects.template get_asset_buffer<Asset>(), &ids };
        context.field(name, _buffer);
    }

    template <typename Asset, typename ArchiveType>
    void save_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        mappings_manager_game_save& _mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
        context_save_storage _context { archive, objects };
        save_asset_buffer_direct<Asset>(_context, "assets", objects, _mappings.objects.user_assets.template get_mapping<Asset>());
    }

    template <typename Asset, typename ArchiveType>
    void load_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
        context_load_storage _context { archive, objects, _mappings.loading_window };
        load_asset_buffer_direct<Asset>(_context, "assets", objects, _mappings.objects.user_assets.template get_mapping<Asset>());
        _context.close();
    }

#define LUCARIA_DETAIL_FOR_EACH_BUILTIN_ASSET(X)           \
    X(asset_image, images, "images")                       \
    X(asset_texture, textures, "textures")                 \
    X(asset_cubemap, cubemaps, "cubemaps")                 \
    X(asset_geometry, geometries, "geometries")            \
    X(object_shape, shapes, "shapes")                      \
    X(asset_mesh, meshes, "meshes")                        \
    X(object_font, fonts, "fonts")                         \
    X(asset_audio, audios, "audios")                       \
    X(object_sound_track, sound_tracks, "sound_tracks")    \
    X(object_skeleton, skeletons, "skeletons")             \
    X(asset_animation, animations, "animations")           \
    X(object_motion_track, motion_tracks, "motion_tracks") \
    X(object_event_track, event_tracks, "event_tracks")

    inline void manager_assets::save(context_save_storage& context)
    {
        mappings_manager_game_save& _mappings = visit_save_archive(context.archive, [](auto& archive) -> mappings_manager_game_save& {
            return cereal::get_user_data<mappings_manager_game_save>(archive);
        });
#define LUCARIA_DETAIL_SAVE_ASSET_BUFFER(Type, Member, Name) save_asset_buffer_direct<Type>(context, Name, *this, _mappings.objects.Member);
        LUCARIA_DETAIL_FOR_EACH_BUILTIN_ASSET(LUCARIA_DETAIL_SAVE_ASSET_BUFFER)
#undef LUCARIA_DETAIL_SAVE_ASSET_BUFFER
        std::vector<storage_user_asset_group> user_assets = {};
        for (const auto& [type_id, callbacks] : user_asset_types) {
            storage_user_asset_group& group = user_assets.emplace_back();
            group.type_id = type_id;
            group.objects = this;
            group.callbacks = &callbacks;
        }
        context.field("user_assets", user_assets);
    }

    inline void manager_assets::load(context_load_storage& context)
    {
        mappings_manager_game_load* _mappings = nullptr;
        visit_load_archive(context.archive, [&](auto& archive) {
            _mappings = &cereal::get_user_data<mappings_manager_game_load>(archive);
        });
        LUCARIA_DEBUG_ASSERT(_mappings != nullptr, "Cannot load assets without an archive-backed context_load_storage");
#define LUCARIA_DETAIL_LOAD_ASSET_BUFFER(Type, Member, Name) load_asset_buffer_direct<Type>(context, Name, *this, _mappings->objects.Member);
        LUCARIA_DETAIL_FOR_EACH_BUILTIN_ASSET(LUCARIA_DETAIL_LOAD_ASSET_BUFFER)
#undef LUCARIA_DETAIL_LOAD_ASSET_BUFFER
        std::vector<storage_user_asset_group> _user_assets = {};
        context.field("user_assets", _user_assets);
    }

#undef LUCARIA_DETAIL_FOR_EACH_BUILTIN_ASSET

    struct snapshot_assets {
        manager_assets& objects;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            context_save_storage _context { archive, objects };
            objects.save(_context);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            mappings_manager_game_load& _mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            LUCARIA_DEBUG_ASSERT(_mappings.loading_objects != nullptr, "Missing manager_assets while loading snapshot assets");
            context_load_storage _context { archive, *_mappings.loading_objects, _mappings.loading_window };
            _mappings.loading_objects->load(_context);
            _context.close();
        }
    };

    // declared in manager_assets.hpp
    template <typename Asset>
    void manager_assets::register_user_asset(std::string type_id)
    {
        user_asset_type_callbacks callbacks = {};
        const auto save = [](manager_assets& objects, auto& archive) {
            save_user_asset_group<Asset>(objects, archive);
        };
        const auto load = [](manager_assets& objects, auto& archive) {
            load_user_asset_group<Asset>(objects, archive);
        };
        callbacks.binary_save = save;
        callbacks.binary_load = load;
        callbacks.json_save = save;
        callbacks.json_load = load;
        user_asset_type_ids[std::type_index(typeid(Asset))] = type_id;
        user_asset_types[std::move(type_id)] = std::move(callbacks);
        get_asset_buffer<Asset>();
    }
}
}
