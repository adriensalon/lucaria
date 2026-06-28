#include <lucaria/core/serialize_mappings.hpp>

namespace lucaria {

#define LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(Asset, SaveMember, LoadMember, NameLiteral)                            \
    template <>                                                                                                    \
    struct detail::handle_asset_mapping<Asset> {                                                                   \
        static uint32 save(detail::mappings_manager_object_save& mappings, const detail::assets_cell<Asset>* cell) \
        {                                                                                                          \
            return mappings.SaveMember.get(cell);                                                                  \
        }                                                                                                          \
        static detail::assets_cell<Asset>* load(detail::mappings_manager_object_load& mappings, const uint32 id)   \
        {                                                                                                          \
            return mappings.LoadMember.get(id);                                                                    \
        }                                                                                                          \
        static constexpr const char* name()                                                                        \
        {                                                                                                          \
            return NameLiteral;                                                                                    \
        }                                                                                                          \
    };
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::asset_animation, animations, animations, "animation")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::asset_audio, audios, audios, "audio")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::asset_cubemap, cubemaps, cubemaps, "cubemap")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::object_event_track, event_tracks, event_tracks, "event_track")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::object_font, fonts, fonts, "font")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::asset_geometry, geometries, geometries, "geometry")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::asset_image, images, images, "image")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::asset_mesh, meshes, meshes, "mesh")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::object_motion_track, motion_tracks, motion_tracks, "motion_track")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::object_shape, shapes, shapes, "shape")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::object_skeleton, skeletons, skeletons, "skeleton")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::object_sound_track, sound_tracks, sound_tracks, "sound_track")
LUCARIA_DEFINE_HANDLE_ASSET_MAPPING(detail::asset_texture, textures, textures, "texture")
#undef LUCARIA_DEFINE_HANDLE_ASSET_MAPPING

}
