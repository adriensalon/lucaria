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
        user_asset_type_callbacks* callbacks = nullptr;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            archive(cereal::make_nvp("type_id", type_id));

            if constexpr (std::is_base_of_v<cereal::JSONOutputArchive, ArchiveType>) {
                callbacks->json_save(*objects, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryOutputArchive, ArchiveType>) {
                callbacks->binary_save(*objects, archive);
            }
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
            if constexpr (std::is_base_of_v<cereal::JSONInputArchive, ArchiveType>) {
                it->second.json_load(*mappings.loading_objects, archive);
            } else if constexpr (std::is_base_of_v<cereal::PortableBinaryInputArchive, ArchiveType>) {
                it->second.binary_load(*mappings.loading_objects, archive);
            }
        }
    };


    template <typename AssetType>
    [[nodiscard]] mappings_container_cache_vector_save<AssetType>& get_asset_mapping(mappings_manager_object_save& mappings);

    template <typename AssetType>
    [[nodiscard]] mappings_container_cache_vector_load<AssetType>& get_asset_mapping(mappings_manager_object_load& mappings);

#define LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(Type, Member) \
    template <> inline mappings_container_cache_vector_save<Type>& get_asset_mapping<Type>(mappings_manager_object_save& mappings) { return mappings.Member; } \
    template <> inline mappings_container_cache_vector_load<Type>& get_asset_mapping<Type>(mappings_manager_object_load& mappings) { return mappings.Member; }

    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(asset_animation, animations)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(asset_audio, audios)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_cubemap, cubemaps)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_event_track, event_tracks)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_font, fonts)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(asset_geometry, geometries)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_image, images)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_mesh, meshes)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_motion_track, motion_tracks)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_shape, shapes)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_skeleton, skeletons)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_sound_track, sound_tracks)
    LUCARIA_DETAIL_DEFINE_ASSET_STORAGE(object_texture, textures)

#undef LUCARIA_DETAIL_DEFINE_ASSET_STORAGE

    template <typename AssetType>
    struct direct_asset_cell_save {
        uint32 save_id = 0;
        AssetType* value = nullptr;

        void save(storage_save_context& context) const
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

        void load(storage_load_context& context)
        {
            context.field("object_save_id", save_id);

            if (buffer == nullptr || ids == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing asset buffer or mapping while loading asset cell");
                return;
            }

            assets_cell<AssetType>* cell = buffer->create_cell(container_async<AssetType>::pending(AssetType {}));
            AssetType& asset = cell->fetched.emplaced_value();

            bool loaded = false;
            std::visit([&](auto archive_value) {
                using archive_value_type = std::decay_t<decltype(archive_value)>;
                if constexpr (!std::is_same_v<archive_value_type, std::monostate>) {
                    using child_archive_type = std::remove_pointer_t<archive_value_type>;
                    std::shared_ptr<load_storage_context<AssetType, child_archive_type>> child_context =
                        std::make_shared<load_storage_context<AssetType, child_archive_type>>(*archive_value, context.objects, cell, asset, context.window());
                    context.objects.active_load_contexts.emplace_back(child_context);
                    load_user_asset_value(asset, *child_context);
                    child_context->close();
                    loaded = true;
                }
            }, context.archive);

            if (!loaded) {
                LUCARIA_DEBUG_ERROR("Cannot load asset cell without an archive-backed storage_load_context");
                return;
            }

            ids->set(save_id, cell);
        }
    };

    template <typename AssetType>
    struct direct_asset_buffer_save {
        manager_assets* objects = nullptr;
        const assets_buffer<AssetType>* buffer = nullptr;
        mappings_container_cache_vector_save<AssetType>* ids = nullptr;

        void save(storage_save_context& context) const
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

        void load(storage_load_context& context)
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
    void save_asset_buffer_direct(storage_save_context& context, std::string_view name, manager_assets& objects, mappings_container_cache_vector_save<AssetType>& ids)
    {
        direct_asset_buffer_save<AssetType> buffer {
            &objects,
            &objects.template get_asset_buffer<AssetType>(),
            &ids
        };
        context.field(name, buffer);
    }

    template <typename AssetType>
    void load_asset_buffer_direct(storage_load_context& context, std::string_view name, manager_assets& objects, mappings_container_cache_vector_load<AssetType>& ids)
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
        storage_save_context context { archive, objects };
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
        storage_load_context context { archive, objects, mappings.loading_window };
        load_asset_buffer_direct<AssetType>(
            context,
            "assets",
            objects,
            mappings.objects.user_assets.template get_mapping<AssetType>());
        context.close();
    }

    inline void manager_assets::save(storage_save_context& context)
    {
        mappings_manager_game_save& mappings = std::visit([](auto* archive) -> mappings_manager_game_save& {
            return cereal::get_user_data<mappings_manager_game_save>(*archive);
        }, context.archive);

        save_asset_buffer_direct<object_image>(context, "images", *this, mappings.objects.images);
        save_asset_buffer_direct<object_texture>(context, "textures", *this, mappings.objects.textures);
        save_asset_buffer_direct<object_cubemap>(context, "cubemaps", *this, mappings.objects.cubemaps);
        save_asset_buffer_direct<asset_geometry>(context, "geometries", *this, mappings.objects.geometries);
        save_asset_buffer_direct<object_shape>(context, "shapes", *this, mappings.objects.shapes);
        save_asset_buffer_direct<object_mesh>(context, "meshes", *this, mappings.objects.meshes);
        save_asset_buffer_direct<object_font>(context, "fonts", *this, mappings.objects.fonts);
        save_asset_buffer_direct<asset_audio>(context, "audios", *this, mappings.objects.audios);
        save_asset_buffer_direct<object_sound_track>(context, "sound_tracks", *this, mappings.objects.sound_tracks);
        save_asset_buffer_direct<object_skeleton>(context, "skeletons", *this, mappings.objects.skeletons);
        save_asset_buffer_direct<asset_animation>(context, "animations", *this, mappings.objects.animations);
        save_asset_buffer_direct<object_motion_track>(context, "motion_tracks", *this, mappings.objects.motion_tracks);
        save_asset_buffer_direct<object_event_track>(context, "event_tracks", *this, mappings.objects.event_tracks);

        std::vector<storage_user_asset_group> user_assets = {};
        for (const auto& [type_id, callbacks] : user_asset_types) {
            storage_user_asset_group& group = user_assets.emplace_back();
            group.type_id = type_id;
            group.objects = this;
            group.callbacks = const_cast<user_asset_type_callbacks*>(&callbacks);
        }
        context.field("user_assets", user_assets);
    }

    inline void manager_assets::load(storage_load_context& context)
    {
        mappings_manager_game_load& mappings = std::visit([](auto archive_value) -> mappings_manager_game_load& {
            using archive_value_type = std::decay_t<decltype(archive_value)>;
            if constexpr (std::is_same_v<archive_value_type, std::monostate>) {
                LUCARIA_DEBUG_ERROR("Cannot load assets without an archive-backed storage_load_context");
                static mappings_manager_game_load dummy = {};
                return dummy;
            } else {
                return cereal::get_user_data<mappings_manager_game_load>(*archive_value);
            }
        }, context.archive);

        load_asset_buffer_direct<object_image>(context, "images", *this, mappings.objects.images);
        load_asset_buffer_direct<object_texture>(context, "textures", *this, mappings.objects.textures);
        load_asset_buffer_direct<object_cubemap>(context, "cubemaps", *this, mappings.objects.cubemaps);
        load_asset_buffer_direct<asset_geometry>(context, "geometries", *this, mappings.objects.geometries);
        load_asset_buffer_direct<object_shape>(context, "shapes", *this, mappings.objects.shapes);
        load_asset_buffer_direct<object_mesh>(context, "meshes", *this, mappings.objects.meshes);
        load_asset_buffer_direct<object_font>(context, "fonts", *this, mappings.objects.fonts);
        load_asset_buffer_direct<asset_audio>(context, "audios", *this, mappings.objects.audios);
        load_asset_buffer_direct<object_sound_track>(context, "sound_tracks", *this, mappings.objects.sound_tracks);
        load_asset_buffer_direct<object_skeleton>(context, "skeletons", *this, mappings.objects.skeletons);
        load_asset_buffer_direct<asset_animation>(context, "animations", *this, mappings.objects.animations);
        load_asset_buffer_direct<object_motion_track>(context, "motion_tracks", *this, mappings.objects.motion_tracks);
        load_asset_buffer_direct<object_event_track>(context, "event_tracks", *this, mappings.objects.event_tracks);

        std::vector<storage_user_asset_group> user_assets = {};
        context.field("user_assets", user_assets);
    }

    struct snapshot_assets {
        manager_assets& objects;

        template <typename ArchiveType>
        void save(ArchiveType& archive) const
        {
            storage_save_context context { archive, const_cast<manager_assets&>(objects) };
            const_cast<manager_assets&>(objects).save(context);
        }

        template <typename ArchiveType>
        void load(ArchiveType& archive)
        {
            mappings_manager_game_load& mappings = cereal::get_user_data<mappings_manager_game_load>(archive);
            if (mappings.loading_objects == nullptr) {
                LUCARIA_DEBUG_ERROR("Missing manager_assets while loading snapshot assets");
                return;
            }

            storage_load_context context { archive, *mappings.loading_objects, mappings.loading_window };
            mappings.loading_objects->load(context);
            context.close();
        }
    };

    // declared in manager_assets.hpp
    template <typename AssetType>
    void manager_assets::register_user_asset(std::string type_id)
    {
        user_asset_type_callbacks callbacks = {};

        callbacks.binary_save = [](manager_assets& objects, cereal::PortableBinaryOutputArchive& archive) {
            save_user_asset_group<AssetType>(objects, archive);
        };

        callbacks.binary_load = [](manager_assets& objects, cereal::PortableBinaryInputArchive& archive) {
            load_user_asset_group<AssetType>(objects, archive);
        };

        callbacks.json_save = [](manager_assets& objects, cereal::JSONOutputArchive& archive) {
            save_user_asset_group<AssetType>(objects, archive);
        };

        callbacks.json_load = [](manager_assets& objects, cereal::JSONInputArchive& archive) {
            load_user_asset_group<AssetType>(objects, archive);
        };

        user_asset_type_ids[std::type_index(typeid(AssetType))] = type_id;
        user_asset_types[std::move(type_id)] = std::move(callbacks);

        get_asset_buffer<AssetType>();
    }
}
}
