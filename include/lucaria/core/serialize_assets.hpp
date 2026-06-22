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
            auto& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            if (mappings.loading_objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_object while loading user asset group");
                return;
            }
            auto it = mappings.loading_objects->user_asset_types.find(type_id);
            if (it == mappings.loading_objects->user_asset_types.end()) {
                LUCARIA_DEBUG_ERROR("Unknown user asset type while loading snapshot");
                return;
            }
            load_registered_object(archive, it->second, *mappings.loading_objects, "user_asset_group", type_id);
        }
    };


    template <typename AssetType>
    struct direct_asset_cell_save {
        uint32 save_id = 0;
        AssetType* value = nullptr;

        void save(context_save_storage& context) const
        {
            context.field("object_save_id", save_id);
            if (value == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing asset value while saving asset cell");
                return;
            }
            save_user_asset_value(*value, context);
        }
    };

    template <typename AssetType>
    struct direct_asset_cell_load {
        uint32 save_id = 0;
        assets_buffer<AssetType>* buffer = nullptr;
        mappings_container_cache_vector_load<AssetType>* ids = nullptr;

        void load(context_load_storage& context)
        {
            context.field("object_save_id", save_id);

            if (buffer == nullptr || ids == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing asset buffer or mapping while loading asset cell");
                return;
            }

            assets_cell<AssetType>* cell = buffer->create_cell(assets_async_slot<AssetType>::pending(AssetType {}));
            AssetType& asset = cell->fetched.emplaced_value();

            const bool loaded = visit_load_archive(context.archive, [&](auto& archive_value) {
                using child_archive_type = std::decay_t<decltype(archive_value)>;
                std::shared_ptr<load_storage_context<AssetType, child_archive_type>> child_context =
                    std::make_shared<load_storage_context<AssetType, child_archive_type>>(archive_value, context.objects, cell, asset, context.window());
                context.objects.active_load_contexts.emplace_back(child_context);
                load_user_asset_value(asset, *child_context);
                child_context->close();
            });

            if (!loaded) {
                LUCARIA_DEBUG_ERROR("Cannot load asset cell without an archive-backed context_load_storage");
                return;
            }

            ids->set(save_id, cell);
        }
    };

    template <typename AssetType>
    struct direct_asset_buffer_save {
        const assets_buffer<AssetType>* buffer = nullptr;
        mappings_container_cache_vector_save<AssetType>* ids = nullptr;

        void save(context_save_storage& context) const
        {
            if (buffer == nullptr || ids == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing asset buffer or mapping while saving asset buffer");
                return;
            }

            std::size_t count = 0;
            buffer->for_each_cell([&](assets_cell<AssetType>& cell) {
                if (cell.fetched.has_value()) {
                    ++count;
                }
            });

            context.field("count", count);

            std::size_t index = 0;
            buffer->for_each_cell([&](assets_cell<AssetType>& cell) {
                if (!cell.fetched.has_value()) {
                    return;
                }

                direct_asset_cell_save<AssetType> asset {
                    ids->get_or_create(&cell),
                    &cell.fetched.value()
                };

                context.field("asset_" + std::to_string(index), asset);
                ++index;
            });
        }
    };

    template <typename AssetType>
    struct direct_asset_buffer_load {
        assets_buffer<AssetType>* buffer = nullptr;
        mappings_container_cache_vector_load<AssetType>* ids = nullptr;

        void load(context_load_storage& context)
        {
            if (buffer == nullptr || ids == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing asset buffer or mapping while loading asset buffer");
                return;
            }

            std::size_t count = 0;
            context.field("count", count);

            for (std::size_t index = 0; index < count; ++index) {
                direct_asset_cell_load<AssetType> asset {
                    0,
                    buffer,
                    ids
                };
                context.field("asset_" + std::to_string(index), asset);
            }
        }
    };

    template <typename AssetType>
    void save_asset_buffer_direct(context_save_storage& context, std::string_view name, manager_assets& objects, mappings_container_cache_vector_save<AssetType>& ids)
    {
        direct_asset_buffer_save<AssetType> buffer {
            &objects.template get_asset_buffer<AssetType>(),
            &ids
        };
        context.field(name, buffer);
    }

    template <typename AssetType>
    void load_asset_buffer_direct(context_load_storage& context, std::string_view name, manager_assets& objects, mappings_container_cache_vector_load<AssetType>& ids)
    {
        direct_asset_buffer_load<AssetType> buffer {
            &objects.template get_asset_buffer<AssetType>(),
            &ids
        };
        context.field(name, buffer);
    }

    template <typename AssetType, typename ArchiveType>
    void save_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        auto& mappings = cereal::get_user_data<mappings_manager_game_save>(archive);
        context_save_storage context { archive, objects };
        save_asset_buffer_direct<AssetType>(
            context,
            "assets",
            objects,
            mappings.objects.user_assets.template get_mapping<AssetType>());
    }

    template <typename AssetType, typename ArchiveType>
    void load_user_asset_group(manager_assets& objects, ArchiveType& archive)
    {
        auto& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
        context_load_storage context { archive, objects, mappings.loading_window };
        load_asset_buffer_direct<AssetType>(
            context,
            "assets",
            objects,
            mappings.objects.user_assets.template get_mapping<AssetType>());
        context.close();
    }

#define LUCARIA_DETAIL_FOR_EACH_BUILTIN_ASSET(X) \
    X(asset_image, images, "images") \
    X(asset_texture, textures, "textures") \
    X(asset_cubemap, cubemaps, "cubemaps") \
    X(asset_geometry, geometries, "geometries") \
    X(object_shape, shapes, "shapes") \
    X(asset_mesh, meshes, "meshes") \
    X(object_font, fonts, "fonts") \
    X(asset_audio, audios, "audios") \
    X(object_sound_track, sound_tracks, "sound_tracks") \
    X(object_skeleton, skeletons, "skeletons") \
    X(asset_animation, animations, "animations") \
    X(object_motion_track, motion_tracks, "motion_tracks") \
    X(object_event_track, event_tracks, "event_tracks")

    inline void manager_assets::save(context_save_storage& context)
    {
        mappings_manager_game_save& mappings = visit_save_archive(context.archive, [](auto& archive) -> mappings_manager_game_save& {
            return cereal::get_user_data<mappings_manager_game_save>(archive);
        });

#define LUCARIA_DETAIL_SAVE_ASSET_BUFFER(Type, Member, Name) \
        save_asset_buffer_direct<Type>(context, Name, *this, mappings.objects.Member);
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
        mappings_manager_game_load* mappings = nullptr;
        visit_load_archive(context.archive, [&](auto& archive) {
            mappings = &cereal::get_user_data<mappings_manager_game_load>(archive);
        });
        if (mappings == nullptr) {
            LUCARIA_DEBUG_ERROR("Cannot load assets without an archive-backed context_load_storage");
            return;
        }

#define LUCARIA_DETAIL_LOAD_ASSET_BUFFER(Type, Member, Name) \
        load_asset_buffer_direct<Type>(context, Name, *this, mappings->objects.Member);
        LUCARIA_DETAIL_FOR_EACH_BUILTIN_ASSET(LUCARIA_DETAIL_LOAD_ASSET_BUFFER)
#undef LUCARIA_DETAIL_LOAD_ASSET_BUFFER

        std::vector<storage_user_asset_group> user_assets = {};
        context.field("user_assets", user_assets);
    }

#undef LUCARIA_DETAIL_FOR_EACH_BUILTIN_ASSET

    struct snapshot_assets {
        manager_assets& objects;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            context_save_storage context { archive, objects };
            objects.save(context);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            if (mappings.loading_objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_assets while loading snapshot assets");
                return;
            }

            context_load_storage context { archive, *mappings.loading_objects, mappings.loading_window };
            mappings.loading_objects->load(context);
            context.close();
        }
    };

    // declared in manager_assets.hpp
    template <typename AssetType>
    void manager_assets::register_user_asset(std::string type_id)
    {
        user_asset_type_callbacks callbacks = {};

        const auto save = [](manager_assets& objects, auto& archive) {
            save_user_asset_group<AssetType>(objects, archive);
        };
        const auto load = [](manager_assets& objects, auto& archive) {
            load_user_asset_group<AssetType>(objects, archive);
        };

        callbacks.binary_save = save;
        callbacks.binary_load = load;
        callbacks.json_save = save;
        callbacks.json_load = load;

        user_asset_type_ids[std::type_index(typeid(AssetType))] = type_id;
        user_asset_types[std::move(type_id)] = std::move(callbacks);

        get_asset_buffer<AssetType>();
    }
}
}
